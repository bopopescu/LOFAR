/* Copyright � 2000
Michael Gradman and Corwin Joy

Permission to use, copy, modify, distribute and sell this software and
its documentation for any purpose is hereby granted without fee, provided
that the above copyright notice appears in all copies and that both that
copyright notice and this permission notice appear in supporting documentation.
Corwin Joy and Michael Gradman make no representations about the suitability
of this software for any purpose.
It is provided "as is" without express or implied warranty.
*/
// Edited: 03/24/2004 - Alexander Motzkau, BoundIO remembers variant_row index
#include "BoundIO.h"
#include "clib_fwd.h"
#include "DBException.h"
#include "DBStmt.h"
#include "string_util.h"

#include "std_warn_off.h"

#ifdef  WIN32
#ifdef WIN32
	#ifndef DTL_USE_MFC
		#include <windows.h>
	#else 
		#include <afx.h>
	#endif
#endif

#endif
#include <sql.h>
#include <sqlext.h>

#include <sstream>
#include <string>
#include <set>
#include <vector>
#include <cassert>

#include "std_warn_on.h"

#ifdef POSTGRES_BLOB_PATCH
#include <cmath>
#endif

// Reviewed: 11/12/2000 - CJ
// Edited: 12/19/2000 - MG - added namespaces
// Edited: 10/26/2003 - Paul Grenyer http://www.paulgrenyer.co.uk, added static_cast as required by MSVC 7.1
BEGIN_DTL_NAMESPACE

#ifdef POSTGRES_BLOB_PATCH
// Postgres doesn't support BLOB-fields: blobs are closely coupled to
// files in postgres, and stored in a seperate table with special
// io-functions.
// This patch makes it possible to store BLOBs in TEXT fields in Postgres.
// A TEXT field can store everything except a null-byte.
// Two conversion routines are made to convert raw-blobs to postgres-'blobs'
// and viceversa. In a Postgres-blob the null-bytes are converted to 0xB1
// bytes, real 0xB1 bytes are escaped.
// Note: 0xB1 is a byte that is not often used in e.g. executable so this
// byte was choosen as a null-byte substitute.
//
const unsigned char		LUB = 0xB1;			// least used byte
const unsigned char		ESC = 0xAD;			// used as escape character
const unsigned char		RNB = '\0';			// real null byte

void DTLblob2Postgres(blob&	dest, const blob& org) {
	int				org_sz = org.size();

	dest.reserve((long) floor(1.1 * org_sz));		// pre-size larger than original

	dest.append(&ESC, 1);							// ESC byte first
	dest.append(&LUB, 1);							// LUB byte secondly
	
	const unsigned char		*p = org.data();		// append cvt data
	for (int i = 0; i < org_sz; ++i) {	// scan whole original
		switch (*(p+i)) {
		case RNB:						// 0 --> LUB
			dest.append(&LUB, 1);
			break;
		case LUB:						// LUB --> ESC LUB
			dest.append(&ESC, 1);
			dest.append(&LUB, 1);
			break;
		case ESC:						// ESC --> ESC ESC
			dest.append(&ESC, 1);
			dest.append(&ESC, 1);
			break;
		default:
			dest.append(p+i, 1);
		}
	}
}

void Postgresblob2DTL(blob&	dest, const blob& org) {
	int				org_sz = org.size();

	dest.reserve(org_sz);						// pre-size to original size
	
	const unsigned char		*p = org.data();
	const unsigned char		esc = *(p++);		// strip off esc-byte
	const unsigned char		lub = *(p++);		// strip off lub-byte
	org_sz -= 2;								// correct total size

	for (int i = 0; i < org_sz; ++i) {		// scan whole original
		unsigned char	c = *(p+i);
		if (c == esc) {
			// ESC ESC --> ESC and ESC LUB --> LUB
			if ((*(p+i+1) == esc) || (*(p+i+1) == lub)) {
				++i;						// skip over ESC
				dest.append(p+i, 1);
			}
		} else 
		if (c == lub) {						// LUB --> 0
			dest.append(&RNB, 1);
		} else {
			dest.append(&c, 1);
		}
	}
}
#endif
// ************ Implementation code for BoundIO ****************


//We need MINSTRBUFLEN to be 255 at most, otherwise Access97 will give error
//_TEXT("Invalid precision or scale value") since 255 is the largest length text
//field it allows. See SQLBindParam() for details

// Default maximum string length for SQL Server / Access when binding std::string
// Note, if you want to bind longer strings than this, no problem,
// just call str.reserve(MAX SIZE YOU NEED) before you bind your member
// field in your BCA (or you can call BoundIO::SetColumnSize to set the maximum size
// if you don't want to reserve string capacity).  Oracle, and other databases don't have this problem
// since they ignore the ColumnSize argument in SQLBindParameter when using
// SQLPutData.
const size_t MAX_SQL_SERVER_DEFAULT_STRING = 4096;

// Oracle varchar is 2000 .. thus use it for SQL Server and Oracle
const size_t MAX_DEFAULT_STRING = 2000;

// Access maximum string size is 254
const size_t MAX_ACCESS_DEFAULT_STRING = 254;

// Buffer size used for transferring string data via PutData and GetData
// Make sure this buffer size is an even number or BoundIO::MoveRead will have
// problems in SQL Server

const size_t BoundIO::MINSTRBUFLEN = 1024; // if you frequently transfer large blobs or strings, you may want to enlarge this


const SDWORD BoundIO::lenAtExec = SQL_LEN_DATA_AT_EXEC(0); // needed for PutData()

BoundIO BoundIO::CharBind(tchar_struct memberToBind) {
    // Much like Generic bind, but set size to GetColumnSize() and
    // don't use & in InitFromField

    // common tasks for BIND_ADDRESSES and BIND_AS_INDICES

    // use RTTI to get the type of the object
    const STD_::string RTTI = DTL_TYPEID_NAME (memberToBind.buf);

	ETI_Map &SQL_types_to_C = GetSQL_types_to_C();
	TypeTranslation &tt = SQL_types_to_C[RTTI];
	tt.size = memberToBind.size;  // we need to reset size here to handle char[]

	void *base_addr;
	size_t base_size;

	if (pBoundIOs)
	{
		base_addr = pBoundIOs->GetWorkingAddr();
		base_size = pBoundIOs->GetWorkingObjSize();
	}
	else
	{
		base_addr = addr;
		base_size = tt.size;
	}

	// exception safety handled by InitFromField
	InitFromField(tt, memberToBind.buf, base_addr, base_size);
	return *this;
}

BoundIO BoundIO::operator==(tchar_struct memberToBind) {
     paramType = SQL_PARAM_INPUT_OUTPUT;
     return CharBind(memberToBind);
}

BoundIO BoundIO::operator<<(tchar_struct memberToBind) {
     paramType = SQL_PARAM_INPUT;
     return CharBind(memberToBind);
}

BoundIO BoundIO::operator>>(tchar_struct memberToBind) {
     paramType = SQL_PARAM_OUTPUT;
     return CharBind(memberToBind);
}
	
size_t BoundIO::GetColumnSize() const { return size; }

// get the actual pointer we need to bind to
void *BoundIO::GetRawPtr() const
{
    	// use like a case statement
    	// add other types that need special handling here
    	if (IsPrimitive)
    		return addr;   // can use object address directly for primitives
    	else
    		return const_cast<void *>(reinterpret_cast<const void *>(strbuf.get())); // address of buffer for complex types
}

// get size of raw buffer
SDWORD BoundIO::GetRawSize() const
{
    	// use like a case statement
    	// add other types that need special handling here

    	if (IsPrimitive)
    		return size;    // use bound object directly for primitives
    	else
    		return static_cast<SDWORD>(strbuf.size());	// use strbuf size for complex types
}

// get value for precision
SDWORD BoundIO::GetPrecision() const
{
    assert(sqlType != SQL_NUMERIC && sqlType != SQL_DECIMAL);
    // SQL_NUMERIC & SQL_DECIMAL need to specify the precision to use.
    // we don't use these for now, so don't care.  Above assert is just to protect
    // us against future changes

    // add other types that need special handling here, e.g. SQL_NUMERIC and SQL_DECIMAL
    switch(typeId) {
    case C_TIMESTAMP: case C_JTIME_C:
    	return 0; // The precision below seconds at which to report
    default:
    	return 0;
    }
}

// get SQL column size needed for SQLBindParam
// this is *NOT* the C size in bytes but is rather the required size
// of the data field for SQLBindParam purposes.  See ODBC docs on Column Size for details.
SDWORD BoundIO::GetColumnSize(const DBConnection &conn) const
{
    assert(sqlType != SQL_NUMERIC && sqlType != SQL_DECIMAL);
    // SQL_NUMERIC & SQL_DECIMAL need to specify the column width to use.
    // we don't use these for now, so don't care.  Above assert is just to protect
    // us against future changes

    // DLP: added case for std::wstring 12/6/2001
    switch(typeId) {
    case C_TIMESTAMP: case C_JTIME_C:
      return 19; // The number of characters in the yyyy-mm-dd hh:mm:ss format
    case C_CHAR_STAR: case C_WCHAR_STAR: case C_TCSTRING:
      return size; // Character types should array size
    case C_STRING: case C_WSTRING: case C_BLOB:
      if (conn.GetDBMSEnum() == DBConnection::DB_ACCESS)
	return static_cast<SDWORD>( DTL_MAX(size_t(size), MAX_ACCESS_DEFAULT_STRING) );
      else
	// Character types should use max(string capacity, sql server
	// minimum default)
	return static_cast<SDWORD>( DTL_MAX(size_t(size), MAX_DEFAULT_STRING) );
    case C_BOOL:
      return 1; // Character types should use buffer length
    };

    // some numeric types require that their size be specified
    switch(sqlType) {
    case SQL_FLOAT: case SQL_DOUBLE:
    	return 15; // default size for ODBC
    case SQL_REAL:
    	return 7; // default size for ODBC
    default:
    	return 0; // parameter is ignored by ODBC
    }
}

// form of CopyMember() for variant
void BoundIO::CopyVariantRowMember(variant_row &vr, const dtl_variant_t &m)
{
    vr[name] = m;
}

BoundIO::BoundIO() : addr(NULL), offset(0), sqlType(0), cType(0), paramType(SQL_PARAM_INPUT), name(_TEXT("")), typeId(0), IsPrimitive(false), size(0),
    	bufferLength(0), bytesFetched(), // mode(BIND_ADDRESSES),
    	VariantRowIdx(-1), pBoundIOs(NULL), bindingType(UNKNOWN), colNo(0), strbuf()
{ }

// BoundIO::BoundIO(const tstring &nm, BoundColMode bcMode, BoundType bt, BoundIOs &parentCollection) :
BoundIO::BoundIO(const tstring &nm, BoundType bt, BoundIOs &parentCollection) :
    		addr(NULL), offset(0), // mode(bcMode),
    		sqlType(0), cType(0), paramType(SQL_PARAM_INPUT), name(nm), typeId(0), IsPrimitive(false),
    		size(0), bufferLength(0), bytesFetched(), VariantRowIdx(-1), pBoundIOs(&parentCollection), bindingType(bt), colNo(0),
    		strbuf()
    		
{ }

BoundIO::BoundIO(const tstring &nm, BoundType bt) :
    		addr(NULL), offset(0), // mode(bcMode),
    		sqlType(0), cType(0), paramType(SQL_PARAM_INPUT), name(nm), typeId(0), IsPrimitive(false),
    		size(0), bufferLength(0), bytesFetched(), VariantRowIdx(-1), pBoundIOs(NULL), bindingType(bt), colNo(0),
    		strbuf()
{ }

BoundIO::BoundIO(BoundIOs &parentCollection, const tstring &nm, const TypeTranslation &tt, void *field, void *base_addr, size_t base_size) :
    		addr(NULL), offset(0), // mode(BIND_ADDRESSES),
    		sqlType(0), cType(0), paramType(SQL_PARAM_INPUT), name(nm), typeId(0), IsPrimitive(false),
    		size(0), bufferLength(0), bytesFetched(), VariantRowIdx(-1), pBoundIOs(&parentCollection),
    		bindingType(COLUMN), colNo(0),
    		strbuf()
    	{

    		InitFromField(tt, field, base_addr, base_size);
    	}

BoundIO::BoundIO(const BoundIO &boundIO) : addr(boundIO.addr), offset(boundIO.offset), // mode(boundIO.mode),
    	sqlType(boundIO.sqlType), cType(boundIO.cType), paramType(boundIO.paramType), name(boundIO.name), typeId(boundIO.typeId), IsPrimitive(boundIO.IsPrimitive), size(boundIO.size),
    	bufferLength(boundIO.bufferLength), bytesFetched(boundIO.bytesFetched), VariantRowIdx(boundIO.VariantRowIdx), pBoundIOs(boundIO.pBoundIOs),
    	bindingType(boundIO.bindingType), colNo(boundIO.colNo),
	strbuf(boundIO.strbuf)
    	
{}

// nothrow swap
void BoundIO::swap(BoundIO &other) {
    	STD_::swap(addr, other.addr);
    	STD_::swap(sqlType, other.sqlType);
    	STD_::swap(cType, other.cType);
    	STD_::swap(paramType, other.paramType);
    	name.swap(other.name);
    	STD_::swap(size, other.size);
    	STD_::swap(bufferLength, other.bufferLength);
    	STD_::swap(bytesFetched, other.bytesFetched);
    	STD_::swap(bindingType, other.bindingType);
    	STD_::swap(colNo, other.colNo);
    	// STD_::swap(mode, other.mode);
    	STD_::swap(offset, other.offset);
    	STD_::swap(pBoundIOs, other.pBoundIOs);
    	STD_::swap(VariantRowIdx, other.VariantRowIdx);
    	strbuf.swap(other.strbuf);
    	STD_::swap(typeId, other.typeId);
    	STD_::swap(IsPrimitive, other.IsPrimitive);
    }

// exception safe assignment
BoundIO &BoundIO::operator=(const BoundIO &boundIO)
{

       if (this != &boundIO)
       {
    	   BoundIO temp(boundIO);
    	   swap(temp);

       }

       return *this;
}


// BoundIO::BoundColMode BoundIO::GetBoundColMode() const { return mode; }

BoundIO::BoundType BoundIO::GetBindingType() const { return bindingType; }

void BoundIO::SetBindingType(BoundIO::BoundType bt) { bindingType = bt; }

int BoundIO::GetColNo() const {return colNo;}

void BoundIO::SetColNo(int iCol) {colNo = iCol;}

bool BoundIO::IsParam()  const { return bindingType == PARAM;   }

bool BoundIO::IsColumn() const { return bindingType == COLUMN;  }

bool BoundIO::IsKnown() const { return bindingType != UNKNOWN; }

// returns whether the column fetched is null
bool BoundIO::IsNull() const { return *bytesFetched == SQL_NULL_DATA; }

// set the given column to represent a NULL value for writing parameters
void BoundIO::SetNull() { *bytesFetched = SQL_NULL_DATA; }

// clear the NULL status given above
// indicator must be SQL_NTS for strings and 0 otherwise
// except for strings, all SQL_C types have fixed lengths
void BoundIO::ClearNull() {
	if (typeId != C_CHAR_STAR && typeId != C_WCHAR_STAR && typeId != C_TCSTRING)
    	*bytesFetched = SDWORD(0);
	else
		*bytesFetched = SDWORD(SQL_NTS);
}

// set the field for this BoundIO to NULL if a column and primitive
void BoundIO::InitNullField()
{
	if (IsNull() && (IsPrimitiveColumn() || IsJtime()) && IsColumn())
		std_memset(addr, 0, size);
}

// does this BoundIO represent a string?
bool BoundIO::IsString() const
{
    return ((typeId == C_WSTRING) || (typeId == C_STRING)
	    || (typeId == C_BLOB) || (typeId == C_CHAR_STAR)
	    || (typeId == C_WCHAR_STAR)
		|| (typeId == C_TCSTRING)
	    );
}

// does this BoundIO represent a jtime?
bool BoundIO::IsJtime() const
{
    return (typeId == C_JTIME_C);
}

// does this BoundIO represent a variable length string?
bool BoundIO::IsVariString() const
{
    return ((typeId == C_STRING) || (typeId == C_WSTRING) || (typeId == C_BLOB));
}

// is this an array_string
bool BoundIO::IsCString() const
{
	return (typeId == C_TCSTRING);
}

// overloads needed for variants to correctly handle null fields
int BoundIO::Compare(variant_row *key1, variant_row *key2) const
{
    	// first we must check for NULL valued fields
    	// NULL fields always equal to one another
    	// NULL is treated as being less than a non-NULL
    	if (key1->IsNull(name) && key2->IsNull(name))
    		return 0;  // NULL == NULL
    	else if (key1->IsNull(name) && !key2->IsNull(name))
    		return -1; // NULL < non-NULL
    	else if (!key1->IsNull(name) && key2->IsNull(name))
    		return 1;  // non-NULL > NULL

    	// both fields non-NULL - do normal check

    	// offset into the objects should be valid
    	// as we checked the offset back in BoundIOs::operator==()
    	// so no check needs to be done here

    	// get pointer to the actual member in each DataObj
    	// now using our BoundIO offset
    	void *pMember1 = (void *) (data_ptr(key1) + offset);
    	void *pMember2 = (void *) (data_ptr(key2) + offset);

    	// now we must cast to the appropriate type based on the
    	// type name of the BoundIO and then perform the comparison
    	return GenericCmp(pMember1, pMember2, typeId);
}

size_t BoundIO::Hash(variant_row *key1) const
{
    	// null fields --> return identity element
    	if (key1->IsNull(name))
    		return 0;

    	// offset into the objects should be valid
    	// as we checked the offset back in BoundIOs::operator==()
    	// so no check needs to be done here

    	// get pointer to the actual member in each DataObj
    	// now using our BoundIO offset
    	void *pMember1 = (void *) (data_ptr(key1) + offset);

    	// now we must cast to the appropriate type based on the
    	// type name of the BoundIO and then perform the hash
    	if (IsNull())
    		return 0;  // null fields hash to the identity element
    	else
    		return GenericHash(pMember1, typeId);
}

// needed for proper STL tstring handling
// MoveRead() and MoveWrite()
// the functions that propagate the data back to the STL tstring
// upon reading or writing respectively from/to the database
void BoundIO::MoveRead(DBStmt &stmt)
{
    	if (IsParam())
    		return;

		// add support for other C types below
		// such as blobs, etc.
		// as in a case statement ... test against typeId
		if (typeId == C_STRING)
		{

			// type is string, cast is OK
			STD_::string *perm_str_ptr = (STD_::string *) addr;

			// make a temp string for exception safety
			// in case of NULL data return empty string
			STD_::string tmp;


			SDWORD bufferSize, bytesGet;

			SDWORD lastFetched = 0;

			bufferSize = GetRawSize() - 1;

			bool first_time = true;

#ifdef DTL_NO_SQL_GETDATA
			while(first_time) {
#else

			// For long strings, continue to grab chunks until no more left.
			// Only call GetData a second time if results from the first call could
			// not completely fit in the buffer.
			while (stmt.GetData(GetColNo(),GetCType(), GetRawPtr(),
				GetRawSize(), GetBytesFetchedPtr()) != DBStmt::NO_DATA_FETCHED)  {
#endif

				if (first_time)
				{
					first_time = false;
					if (*bytesFetched <= 0) break;
					bytesGet = DTL_MIN(bufferSize, *bytesFetched);

					tmp.reserve(*bytesFetched); // pre-size with total # of bytes
					tmp.append(reinterpret_cast<char *>(strbuf.get()), bytesGet);
					lastFetched = *bytesFetched;
				}
				else
				{
					// make sure that bytesGet <= bufferSize.  e.g. Access will lie about the
					// total size of bytes it will return in the first setting for lastFetched
					// and overflow the buffer for strings longer than 2046
					bytesGet = DTL_MIN(DTL_MIN(lastFetched - *bytesFetched, *bytesFetched), bufferSize);

					// bytesGet = DTL_MIN(bufferSize, DTL_MAX(*bytesFetched, SDWORD(0)));
					if (bytesGet <= 0) break;

					tmp.append(reinterpret_cast<char *>(strbuf.get()), bytesGet);
					lastFetched = *bytesFetched;
				}
			}

			// handles three cases:
			// 1. Data in parts (mutiple iterations in the above while loop).
			// 2. NULL data - will set *GetBytesFetchedPtr() to SQL_NULL_DATA automatically
			// 3. tmp = "\0"- will set *GetBytesFetchedPtr() to 0 automatically
			if (tmp.size() > 0)
				*GetBytesFetchedPtr() = static_cast<SDWORD>( tmp.size() );


			perm_str_ptr->swap(tmp); // now we can commit the results to memory

		}

		else if (typeId == C_BLOB)
		{

			// type is string, cast is OK
			blob *perm_str_ptr = (blob *) addr;

			// make a temp string for exception safety
			// in case of NULL data return empty string
			blob tmp;


			SDWORD bufferSize, bytesGet;

			SDWORD lastFetched = 0;

			bufferSize = GetRawSize();


			bool first_time = true;

#ifdef DTL_NO_SQL_GETDATA
			while(first_time) {
#else
			// For long strings, continue to grab chunks until no more left.
			// Only call GetData a second time if results from the first call could
			// not completely fit in the buffer.
			while (stmt.GetData(GetColNo(),GetCType(), GetRawPtr(),
				GetRawSize(), GetBytesFetchedPtr()) != DBStmt::NO_DATA_FETCHED)  {
#endif

				if (first_time)
				{
					first_time = false;
					if (*bytesFetched <= 0) break;
					bytesGet = DTL_MIN(bufferSize, *bytesFetched);

					tmp.reserve(*bytesFetched); // pre-size with total # of bytes
					tmp.append(reinterpret_cast<BYTE *>(strbuf.get()), bytesGet);
					lastFetched = *bytesFetched;
				}
				else
				{
					// make sure that bytesGet <= bufferSize.  e.g. Access will lie about the
					// total size of bytes it will return in the first setting for lastFetched
					// and overflow the buffer for strings longer than 2046
					bytesGet = DTL_MIN(DTL_MIN(lastFetched - *bytesFetched, *bytesFetched), bufferSize);

					// bytesGet = DTL_MIN(bufferSize, DTL_MAX(*bytesFetched, SDWORD(0)));
					if (bytesGet <= 0) break;

					tmp.append(reinterpret_cast<BYTE *>(strbuf.get()), bytesGet);
					lastFetched = *bytesFetched;
				}
			}

			// handles three cases:
			// 1. Data in parts (mutiple iterations in the above while loop).
			// 2. NULL data - will set *GetBytesFetchedPtr() to SQL_NULL_DATA automatically
			// 3. tmp = "\0"- will set *GetBytesFetchedPtr() to 0 automatically
			if (tmp.size() > 0)
				*GetBytesFetchedPtr() = static_cast<SDWORD>( tmp.size() );

#ifdef POSTGRES_BLOB_PATCH
			blob	converted_blob;
			Postgresblob2DTL(converted_blob, tmp);
			perm_str_ptr->swap(converted_blob);
#else
			perm_str_ptr->swap(tmp); // now we can commit the results to memory
#endif

		}

#ifndef DTL_NO_UNICODE
		// DLP: added to support std::wstring
		else if (typeId == C_WSTRING)
		{
			// type is string, cast is OK
			STD_::wstring *perm_str_ptr = (STD_::wstring *) addr;

			// make a temp string for exception safety
			// in case of NULL data return empty string
			STD_::wstring tmp;

			SDWORD bufferSize, bytesGet;

			SDWORD lastFetched = 0;

			bufferSize = GetRawSize() - 1;

			bool first_time = true;

#ifdef DTL_NO_SQL_GETDATA
			while(first_time) {
#else
			// For long strings, continue to grab chunks until no more left.
			// Only call GetData a second time if results from the first call could
			// not completely fit in the buffer.
			while (stmt.GetData(GetColNo(),GetCType(), GetRawPtr(),
				GetRawSize(), GetBytesFetchedPtr()) != DBStmt::NO_DATA_FETCHED)  {
#endif

				if (first_time)
				{
					first_time = false;
					if (*bytesFetched <= 0) break;
					bytesGet = DTL_MIN(bufferSize, *bytesFetched);

					tmp.reserve(*bytesFetched / sizeof(wchar_t)); // pre-size with total # of bytes
					tmp.append(reinterpret_cast<wchar_t *>(strbuf.get()),
						bytesGet / sizeof(wchar_t));
					lastFetched = *bytesFetched;
				}
				else
				{
					// make sure that bytesGet <= bufferSize.  e.g. Access will lie about the
					// total size of bytes it will return in the first setting for lastFetched
					// and overflow the buffer for strings longer than 2046
					bytesGet = DTL_MIN(DTL_MIN(lastFetched - *bytesFetched, *bytesFetched), bufferSize);

					// bytesGet = DTL_MIN(bufferSize, DTL_MAX(*bytesFetched, SDWORD(0)));
					if (bytesGet <= 0) break;

					tmp.append(reinterpret_cast<wchar_t *>(strbuf.get()),
						bytesGet / sizeof(wchar_t));
					lastFetched = *bytesFetched;
				}
			}

			// handles three cases:
			// 1. Data in parts (mutiple iterations in the above while loop).
			// 2. NULL data - will set *GetBytesFetchedPtr() to SQL_NULL_DATA automatically
			// 3. tmp = "\0"- will set *GetBytesFetchedPtr() to 0 automatically
			if (tmp.size() > 0)
				*GetBytesFetchedPtr() = static_cast<SDWORD>( tmp.size() * sizeof(wchar_t) );


			perm_str_ptr->swap(tmp); // now we can commit the results to memory

		}
#endif

		else
		if (typeId == C_JTIME_C)
		{

			// make sure date is not a SQL null

			if (!IsNull())
			{
				// copy timestamp structure;
				jtime_c tmp ( *((timestamp_t *)strbuf.get()));
				jtime_c *perm_ptr = (jtime_c *) addr;
				*perm_ptr = tmp;
			}
			//else
				// simply put empty date here - up to SelValidate to decide what to do
		}
}

template<typename T> static SQLINTEGER do_move_write_len (void* addr, T *dummy)
{
  T* perm_str_ptr = reinterpret_cast<T*> (addr);
  return static_cast<SQLINTEGER>( perm_str_ptr->length ( ) );
}

void BoundIO::MoveWrite(SQLQueryType sqlQryType, DBStmt &stmt)
{
    if (IsColumn() && sqlQryType == SELECT)
    	return;

    // add support for other C types below such as blobs, etc.  as
    // in a case statement ... test against typeId

    // still have to handle dates
    if (typeId == C_JTIME_C) 
	{
		timestamp_t *dest;
		jtime_c *src;
		dest = (timestamp_t *) strbuf.get();
		src = (jtime_c *) addr;
		*dest = (timestamp_t)(*src);
    }

	// set string lengths for types that use SQLPutData
	else if (typeId == C_STRING)
		*(GetLenAtExecPtr()) = 
			SQL_LEN_DATA_AT_EXEC(do_move_write_len<STD_::string> (addr, (STD_::string *) NULL));
 
	else if (typeId == C_BLOB)
		*(GetLenAtExecPtr()) = 
			SQL_LEN_DATA_AT_EXEC(do_move_write_len<blob> (addr, (blob *) NULL));
 
#ifndef DTL_NO_UNICODE
	else if (typeId == C_WSTRING)
		*(GetLenAtExecPtr()) = SQL_LEN_DATA_AT_EXEC(do_move_write_len<STD_::wstring> (addr, (STD_::wstring *)NULL));
#endif
}

// comparison operator for BoundIO objects ...
// needed by BoundIOs::operator[]()
bool operator==(const BoundIO &bound1, const BoundIO &bound2)
{
       return (bound1.addr == bound2.addr &&
    		   bound1.bindingType == bound2.bindingType &&
    		   bound1.cType == bound2.cType &&
    		   bound1.sqlType == bound2.sqlType &&
    		   bound1.name == bound2.name &&
    		   bound1.typeId == bound2.typeId);
}



// assuming buffer length and raw size are the same
SDWORD BoundIO::GetBufferLength() const
{
    return GetRawSize();
}

void BoundIO::SetBufferLength(SDWORD sz)
{
	bufferLength = sz;
}

SDWORD BoundIO::GetActualBufferLength() const
{
	return bufferLength;
}

// get C Type
SDWORD BoundIO::GetCType() const { return cType; }

// get Input/Ouput parameter type
SDWORD BoundIO::GetParamType() const { return paramType;}

void BoundIO::SetParamType(SDWORD ptype) { paramType = ptype; }

// get SQL Type
SDWORD BoundIO::GetSQLType() const { return sqlType; }

// set SQL Type ... use this method to override the default SQL type used
// vs. your C type
void BoundIO::SetSQLType(SDWORD newSqlType) { sqlType = newSqlType; }

void BoundIO::SetCType(SDWORD newCType) { cType = newCType; }

BoundIO & BoundIO::SetColumnSize(size_t s) 
{
	size = static_cast<SDWORD>( s ); 
	return *this; 
}


// get type ID
int BoundIO::GetTypeID() const { return typeId; }

// get pointer to bytesFetched
SDWORD *BoundIO::GetBytesFetchedPtr() const
{
  return const_cast<SDWORD *>(bytesFetched.get());
}

// set pointer to bytesFetched
void BoundIO::SetBytesFetchedPtr(SDWORD *ptr)
{
  bytesFetched = ptr;
  bytesFetched.DontDelete();
  if (typeId != C_CHAR_STAR && typeId != C_WCHAR_STAR && typeId != C_TCSTRING)
    	*bytesFetched = SDWORD(0);
  else
		*bytesFetched = SDWORD(SQL_NTS);


}

// get pointer to length at exec ptr. for PutData()
SDWORD *BoundIO::GetLenAtExecPtr() const
{
  *(GetBytesFetchedPtr()) = SQL_LEN_DATA_AT_EXEC(0);
  return GetBytesFetchedPtr();
}

// returns name of the bound IO object (column name or parameter #)
tstring BoundIO::GetName() const { return name; }


BoundIOs *BoundIO::GetBoundIOsPtr() const
{
    return (BoundIOs*) pBoundIOs;
}

void *BoundIO::GetAddr() const
{
    return addr;
}

void BoundIO::SetAddr(void *address)
{
    addr = address;
}

void BoundIO::SetStrbufAddr(BYTE *address, size_t sz)
{
    strbuf.set(address, sz);
	strbuf.DontDelete();
}

void BoundIO::SetStrbufAddrOnly(BYTE *address)
{
    strbuf.set_addr_only(address);
	strbuf.DontDelete();
}

// return relative offset from bound DataObj
ptrdiff_t BoundIO::GetOffset() const
{
    return offset;
}

// return relative offset from bound DataObj
void BoundIO::SetOffset(ptrdiff_t diff)
{
    offset = diff;
}

void BoundIO::SetAsParam(int numParam)
{
    	// no special care needed for exception-safety
    	// if assignment of name or before throws, atomicity and
    	// consistency still hold
    	tostringstream numstr;
    	numstr << numParam << STD_::ends;

    	name = numstr.str();
    	// numstr.freeze(false);
    	bindingType = PARAM;
}

BoundIO::~BoundIO()
{}

void BoundIO::TypeTranslationFieldBind(TypeTranslationField &ttf) {

    	ttf.field_name = name; // assign name back into variant row list

    	// exception safe binding
    	BoundIO temp(*this);
    	temp.InitFromField(ttf.tt, NULL, NULL, ttf.base_size);
    	size_t mem_offset = (BYTE *) ttf.field - (BYTE *) ttf.base_addr;

    	// make sure bound data is actually part of the object
    	// it supposedly belongs to
    	// if the address doesn't land within the bounds of the object,
    	// throw an exception
    	if (mem_offset >= ttf.base_size)
    		DTL_THROW DBException(_TEXT("BoundIO::InitFromField()"), _TEXT("Trying to bind index field outside of ")
    				_TEXT("base object bounds!"), NULL, NULL);

    	// everything should be OK otherwise
    	temp.offset = mem_offset;
    	swap(temp);
    	SetAsVariantRow(ttf.field_nr);
}

BoundIO BoundIO::operator==(TypeTranslationField ttf) {
    	paramType = SQL_PARAM_INPUT_OUTPUT;
    	TypeTranslationFieldBind(ttf);
    	return *this;
}

BoundIO BoundIO::operator<<(TypeTranslationField ttf) {
    	paramType = SQL_PARAM_INPUT;
    	TypeTranslationFieldBind(ttf);
    	return *this;
}

BoundIO BoundIO::operator>>(TypeTranslationField ttf) {
    	paramType = SQL_PARAM_OUTPUT;
    	TypeTranslationFieldBind(ttf);
    	return *this;
}

// Construct binding definition based on type & address
void BoundIO::InitFromField(const TypeTranslation &tt, void *field, void *base_addr, size_t base_size)  {

    	BoundIO tmp(*this); // need temporary for exception safety

    	if (pBoundIOs)
    	{
    		tmp.colNo = static_cast<int>( BoundIOs_NumColumns((BoundIOs*) pBoundIOs) - 1 ); // set column order for SQL_iterator
    	}
    	else
    	{
    		tmp.colNo = 0;
    	}

    	// make sure the type is in the ETI map!  If not,
    	// a default TypeTranslation object will be built, with the
    	// TypeComplexity as invalid
    	// if we check the TypeComplexity and it is TYPE_INVALID , we've got an exception!
    	if (tt.complexity == TypeTranslation::TYPE_INVALID)
    	{
    	  tstring errmsg;
		  errmsg.reserve(255);
		  errmsg += _TEXT("Type ");
    	  errmsg += tstring_cast((tstring *)NULL, tt.typeNm);
    	  errmsg += _TEXT(" not found in ETI Map!");
    	  DTL_THROW ETIException(_TEXT("BoundIO::InitFromField()"), errmsg);
    	}

    	// now use our ETI table to get the SQL and C types necessary to populate
    	// boundIO
        tmp.sqlType = tt.sqlType;
    	tmp.cType = tt.cType;
    	tmp.typeId = tt.typeId;
    	tmp.IsPrimitive = tt.IsPrimitive();

    	// fill in the other members of boundIO
    	tmp.size = static_cast<SDWORD>( tt.size );
    	tmp.addr = field;

    	if (tmp.typeId == C_STRING  || tmp.typeId == C_WSTRING || tmp.typeId == C_BLOB) {
    	  if (tmp.strbuf.get() == NULL)
    		  tmp.strbuf.reset(DTL_MAX(tt.size, MINSTRBUFLEN));
    	}
    	else
    	  if (tmp.typeId == C_JTIME_C)
    		  tmp.strbuf.reset(sizeof(timestamp_t));

		switch (tmp.typeId)
		{
			default: tmp.bytesFetched = new SDWORD(0); break;

			case C_CHAR_STAR: case C_WCHAR_STAR:
			case C_TCSTRING: tmp.bytesFetched = new SDWORD(SQL_NTS); break;
		}
    	// need to compute offset if we're binding indices
    	// can provide this functionality though if we called BindAsBase() previously

    	if (base_addr)
    	{
    		// check for possible basic error conditions, throw if either occurs
    		if (!tmp.pBoundIOs)
    			DTL_THROW DBException(_TEXT("BoundIO::InitFromField()"),
    				 _TEXT("This BoundIO is not bound to its parent!"), NULL, NULL);

    		size_t mem_offset = (BYTE *) field - (BYTE *) base_addr;

    		// make sure bound data is actually part of the object
    		// it supposedly belongs to
    		// if the address doesn't land within the bounds of the object,
    		// throw an exception
    		if (mem_offset >= base_size)
    			DTL_THROW DBException(_TEXT("BoundIO::InitFromField()"), _TEXT("Trying to bind index field outside of ")
    					_TEXT("base object bounds!"), NULL, NULL);

    		// everything should be OK otherwise
    		tmp.offset = mem_offset;
    	}

    	swap(tmp); // tmp, process, and swap() guarantee atomicity and consistency
    		       // in the face of exceptions
}

void BoundIO::SetAsVariantRow(int nr)
{
    VariantRowIdx = nr;
}

bool BoundIO::GetIsVariantRow() const
{
	return VariantRowIdx!=-1;
}

int BoundIO::GetVariantRowIdx() const
{
        return VariantRowIdx;
}

tostream &operator<<(tostream &o, const BoundIO &b)
{
    o << _TEXT("Name: ") << b.GetName() << _TEXT(", ");
    if (b.IsColumn())
    	o << _TEXT("ColNo: ") << b.GetColNo() << _TEXT(", ");

    o << _TEXT("Ptr: ") << b.GetRawPtr() << _TEXT(", ");
    if (b.IsNull())
    	o << _TEXT("<NULL>");
    else
    	o << _TEXT("<NOT NULL>");
    return o;
}

// ************* Implementation code for BoundIOs **************

//BoundIOs::BoundIOs(BoundIO::BoundColMode bcMode) :
BoundIOs::BoundIOs() :
    STD_::map<tstring, BoundIO, NonCaseSensitiveLt>(), // mode(bcMode),
    	working_addr(NULL), working_type(""),
    	working_size(0), cColumns(0),pBytesFetchedArray(),
		pRowStatusArray(), pNumRowsFetched(), pDatesFetchedArray() { }

// Should rebind bases for proper behavior after call to copy constructor!
BoundIOs::BoundIOs(const BoundIOs &boundIOs) : STD_::map<tstring, BoundIO, NonCaseSensitiveLt>(boundIOs),
    	// mode(boundIOs.mode),
    	working_addr(boundIOs.working_addr), working_type(boundIOs.working_type),
    	working_size(boundIOs.working_size), cColumns(boundIOs.cColumns),
		pBytesFetchedArray(boundIOs.pBytesFetchedArray), pRowStatusArray(boundIOs.pRowStatusArray),
		pNumRowsFetched(boundIOs.pNumRowsFetched), pDatesFetchedArray(boundIOs.pDatesFetchedArray)
{
    	// update parentCollection pointers
    	for (BoundIOs::iterator b_it = begin(); b_it != end(); b_it++)
    	{
    		BoundIO &boundIO = (*b_it).second;
    		boundIO.pBoundIOs = this;
    	}
}

// no-throw swap
void BoundIOs::swap(BoundIOs &other) {
    	using STD_::map;

    	// STD_::swap(mode, other.mode);
    	working_type.swap(other.working_type);
    	STD_::swap(working_size, other.working_size);
    	STD_::swap(cColumns, other.cColumns);

		pBytesFetchedArray.swap(other.pBytesFetchedArray);
		pRowStatusArray.swap(other.pRowStatusArray);
		pNumRowsFetched.swap(other.pNumRowsFetched);
		pDatesFetchedArray.swap(other.pDatesFetchedArray);

    	map<tstring, BoundIO, NonCaseSensitiveLt>::swap(other);

    	// update parentCollection pointers for this
    	BoundIOs::iterator b_it;
    	for (b_it = begin(); b_it != end(); b_it++)
    	{
    		BoundIO &boundIO = (*b_it).second;
    		boundIO.pBoundIOs = this;
    	}
    	// update parentCollection pointers for other
    	for (b_it = other.begin(); b_it != other.end(); b_it++)
    	{
    		BoundIO &boundIO = (*b_it).second;
    		boundIO.pBoundIOs = &other;
    	}
}

BoundIOs &BoundIOs::operator=(const BoundIOs &boundIOs)
{
    	if (this != &boundIOs)
    	{
    		BoundIOs temp(boundIOs);
    		BoundIOs::swap(temp);
    	}

    	return *this;
}

void BoundIOs::BindAsBase(void *addr, size_t size, STD_::string name) {

    // attempting to assign working_type first guarantees exception safety
    working_type = name;
    working_addr = addr;
    working_size = size;
}

// returns address of current DataObj / ParamObj we are working with in bca / bpa
void *BoundIOs::GetWorkingAddr()
{
    return working_addr;
}

// returns size of current DataObj / ParamObj we are working with in bca / bpa
size_t BoundIOs::GetWorkingObjSize()
{
    return working_size;
}

// returns stringified type of current DataObj / ParamObj we are working with in bca / bpa
STD_::string BoundIOs::GetWorkingObjType()
{
    return working_type;
}

// you must call this function to bind the base address of the DataObj
// just prior to calling the
// return the column with the given name
BoundIO &BoundIOs::operator[](const tstring &colName)
{
      // if element found ... simply return reference to existing object

      // if element not found ... create object the way we want it, insert it into
      // map and then return reference to where the object was inserted
      using STD_::map;

      BoundIO boundIO(colName, BoundIO::COLUMN, *this);

      STD_::pair<map<tstring, BoundIO, NonCaseSensitiveLt>::iterator, bool>
   	loc = insert(STD_::pair<const tstring, BoundIO>(colName, boundIO));

      if (loc.second)
    	  cColumns++;

      return (*(loc.first)).second;
}

// return the parameter with the given index
// (uses similar logic to BoundIO::operator[](tstring)
BoundIO &BoundIOs::operator[](unsigned int paramNum)
{
    	tostringstream numstr;
    	numstr << paramNum << STD_::ends;

    	tstring paramstr = numstr.str();

    	BoundIO boundIO(paramstr, BoundIO::PARAM, *this);

    	using STD_::map;

    	    map<tstring, BoundIO, NonCaseSensitiveLt>::iterator loc = insert(STD_::pair<const tstring, BoundIO>(paramstr, boundIO)).first;

    	return (*loc).second;
}

// accessors for BoundColMode ... BIND_ADDRESSES or
// BIND_ADDRESSES
// BoundIO::BoundColMode BoundIOs::GetBoundColMode() { return mode; }

bool bio_it_cmp(BoundIOs::iterator b_it1, BoundIOs::iterator b_it2)
{
    return (b_it1->second.GetColNo() < b_it2->second.GetColNo());
}

// fix column ordering ... needed if column names lose their zero-indexing
void BoundIOs::FixColNos()
{
    STD_::vector<BoundIOs::iterator> bIO_its;

    // find smallest ColNo to get proper offset
    for (BoundIOs::iterator check_it = begin(); check_it != end(); ++check_it)
    {
    	if (check_it->second.IsColumn())
    	{
    		bIO_its.push_back(check_it);
    	}
    }

    STD_::sort(bIO_its.begin(), bIO_its.end(), bio_it_cmp);
    size_t i;
    for (i = 0; i < bIO_its.size(); ++i)
    {
    	bIO_its[i]->second.SetColNo( static_cast<int>(i) );
    }

    cColumns = static_cast<int>(i);
}

// return the names of the columns bound to
STD_::vector<tstring> BoundIOs::GetColumnNames() const
{
    	// we must reorder column names to make all variable length strings
    	// because GetData()'ed columns must all be of a higher number than bound
    	// columns

    	// 1.  separate columns into those which are not variable length strings
    	// and those which are variable length strings

    	STD_::vector<tstring> colNamesReordered;
    	colNamesReordered.reserve(size());

    	// make vector of column names...
    	// first the non varistrings,
    	// then the varistrings
    	BoundIOs *mutant = const_cast<BoundIOs *>(this);
    	bool bVari = false;
    	int j = 0;
    	for (int i=0; i<2; i++) {
    		for (BoundIOs::iterator bc_it = mutant->begin(); bc_it != mutant->end(); bc_it++)
    		{
    			if ((*bc_it).second.IsColumn())
    			{
    				if ((*bc_it).second.IsVariString() == bVari) {
    					colNamesReordered.push_back((*bc_it).first);
    					// now we must update the column number in each BoundIO
    					(*bc_it).second.SetColNo(j);
    					j++;
    				}

    		    }
    		}
    		bVari = true;
    	}

    	return colNamesReordered;
}

// return the # of parameters bound
int BoundIOs::NumParams() const
{
    	int count = 0;

    	for (BoundIOs::const_iterator b_it = begin(); b_it != end();
    			b_it++)
    	{
    		if ((*b_it).second.IsParam())
    			count++;
    	}
    	return count;

}

// return the # of columns bound
int BoundIOs::NumColumns() const
{
    	return cColumns;
}

// Resets the address of the DataObj we are working with & updates all BoundIOs to
// this new address
int BoundIOs::NumJtimes()
{
		int its = 0;
    	for (BoundIOs::iterator bc_it = begin(); bc_it != end(); bc_it++)
    	{
    		STD_::pair<const tstring, BoundIO> &pr = *bc_it;
			if (pr.second.IsColumn() ) {
    			if (pr.second.IsJtime()) {
					++its;
				}
			}
    	}
		return its;
}

// erase a column by name
void BoundIOs::EraseColumn(tstring &col)
{
	erase(col);
	cColumns--; 
	 
}

// erase all existing columns
void BoundIOs::EraseColumns()
{
    	BoundIOs::iterator b_current;
    	for (BoundIOs::iterator b_it = begin(); b_it != end();)
    	{
    		if ((*b_it).second.IsColumn()) {
    			b_current = b_it;
    			b_it++;
    			erase(b_current);
    		}
    		else
    			b_it++;
    	}
    	cColumns = 0;

}

// return true if any of the parameters are output or input_output parameters
bool BoundIOs::HasOutput() const {
    	for (BoundIOs::const_iterator b_it = begin(); b_it != end();
    			b_it++)
    	{
    		if ((*b_it).second.IsParam()  &&
    			((*b_it).second.GetParamType() == SQL_PARAM_OUTPUT ||
    			(*b_it).second.GetParamType() == SQL_PARAM_INPUT_OUTPUT))
    			return true;
    	}
    	return false;

}

// propagate all bound STL strings to their strbufs
// We make this non exception-safe, but that should be O.K. since BoundIOs merely
// hold temporary buffers anyway. Also, making a temporary copy for exception
// safety here will kill us performance wise.
void BoundIOs::PropagateToSQL(SQLQueryType sqlQryType, DBStmt &stmt)
{
    	bool bFail = false;

    	for (BoundIOs::iterator b_it = begin(); b_it != end(); b_it++) {
    		try {
    			BoundIO &boundIO = b_it->second;
    			boundIO.MoveWrite(sqlQryType, stmt); // individual calls exception-safe
    		} catch(...) {
    			bFail = true;
    		}
    	}
    	if (bFail)
    		DTL_THROW DBException(_TEXT("BoundIOs::PropagateToSQL()"),
    		   _TEXT("Error copying object data in BoundIO column"), NULL, NULL);
}

// find the BoundIO with the paramNum passed in

BoundIO &BoundIOs::GetBoundIOforColumn(SQLPOINTER paramNum)
{
    BoundIOs::iterator b_it = begin();

    // using STD_::distance(begin(), static_cast<size_t>) as the token for the parameter
    STD_::advance(b_it, reinterpret_cast<size_t>(paramNum));

    return b_it->second;
}

// propagation stuff needed after Execute() for PutData()
void BoundIOs::PropagateToSQLAfterExec(SQLQueryType sqlQryType, DBStmt &stmt)
{

    SQLPOINTER pToken;
    RETCODE rc;

    bool needToReturn = true;

    // if no true SQL params bound, we must return
    // or we'll get a function out of sequence error

    // must not call SQLParamData() if:
    // 1. SELECT stmt. or SQL iterator being used as a SELECT has no VariString params
    // 2. All other cases - no VariStrings in use at all

    // handle case #1
    if ((sqlQryType == SELECT) || (sqlQryType == SQL && NumColumns() != 0))
    {
    	const tstring sql = stmt.GetQuery();

    	// no question marks in query means no VariString params
    	if (numOfOccurrences(_TEXT('?'), sql) == 0)
    		return;

    	// if at least one VariString parameter, we must break out of loop
    	// and call SQLParamData() at least once
    	for (BoundIOs::iterator b_it = begin(); b_it != end(); ++b_it)
    	{
    		BoundIO &boundIO = b_it->second;
    		if (boundIO.IsParam() && boundIO.IsVariString())
    		{
    			needToReturn = false;
    			break;
    		}
    	}

    	// no VariString params ... do not call SQLParamData()
    	if (needToReturn)
    		return;
    }
    else // handle case #2
    {
    	const tstring sql = stmt.GetQuery();

    	// no question marks in query means no VariString params
    	if (numOfOccurrences(_TEXT('?'), sql) == 0)
    		return;

    	// if at least one VariString BoundIO, we must break out of loop and
    	// call SQLParamData() at least once
    	for (BoundIOs::iterator b_it = begin(); b_it != end(); ++b_it)
    	{
    		BoundIO &boundIO = b_it->second;
    		if (boundIO.IsVariString())
    		{
    			needToReturn = false;
    			break;
    		}
    	}

    	// no VariString BoundIO's ... do not call SQLParamData()
    	if (needToReturn)
    		return;
    }


    do
    {
      rc = stmt.ParamData(&pToken); // find out which parameter we need to process
    								// from driver

      if (rc == SQL_NEED_DATA)
    	  GetBoundIOforColumn(pToken).MoveWriteAfterExec(sqlQryType, stmt);

    } while (rc == SQL_NEED_DATA);


}

void BoundIO::MoveWriteAfterExec(SQLQueryType sqlQryType, DBStmt &stmt)
{
    	if (IsColumn() && ((sqlQryType == SELECT) || sqlQryType == SQL))
    		return;

    	if (paramType == SQL_PARAM_OUTPUT && sqlQryType == SQL)
    		return;

    	if (!IsVariString())
    		return;

    	if (IsNull()) {
    		stmt.PutData(GetRawPtr(), SQL_NULL_DATA);
    		return;
    	}


    	// add support for other C types below
    	// such as blobs, etc.
    	// as in a case statement ... test against typeId
    	if (typeId == C_STRING)
    	{
  		  // type is string, cast is OK
    	  STD_::string *perm_str_ptr = (STD_::string *) addr;

    	  const size_t total_len = perm_str_ptr->length();

    	  // must handle empty strings, else ParamData() doesn't think
    	  // we're done with the parameter
    	  if (total_len == 0)
    	  {
    		std_strncpy(reinterpret_cast<char *>(strbuf.get()), "\0", 1);
    		stmt.PutData(GetRawPtr(), SQL_NULL_DATA);
    		return;
    	  }

    	  size_t start_pos = 0; // position written up to from *perm_str_ptr
    	  size_t end_pos = 0;   // position for end of next write

    	  // For long strings, continue to grab chunks until no more left.
    	  // Only call PutData a second time if results from the first call could
    	  // not completely fit in the buffer.
    	  while (start_pos < total_len)
    	  {
    		size_t left_to_write = total_len - start_pos;

    		// size_t len_to_copy = (left_to_write <= strbuf.size() - 1 ?
    		//	 left_to_write : strbuf.size() - 1);

    		size_t len_to_copy = (left_to_write <= strbuf.size() ?
    			 left_to_write : strbuf.size());

    		// figure what end position of string we're copying into strbuf
    		end_pos += len_to_copy;

    		// Call SQLPutData on chunk from string
    		//
    		stmt.PutData(reinterpret_cast<const void *>(perm_str_ptr->data() + start_pos), 
				static_cast<SDWORD>( len_to_copy ) );

    		// get new start pos. for next iteration of while loop
    		start_pos = end_pos;
    	  }
    	}

    	else if (typeId == C_BLOB)
    	{
    	  // type is blob, cast is OK
#ifdef POSTGRES_BLOB_PATCH
	      blob	converted_blob;
	      DTLblob2Postgres(converted_blob, *((blob*) addr));
		  blob *perm_str_ptr = &converted_blob;
#else
    	  blob *perm_str_ptr = (blob *) addr;
#endif
    	  const size_t total_len = perm_str_ptr->length();

    	  // must handle empty strings, else ParamData() doesn't think
    	  // we're done with the parameter
    	  if (total_len == 0)
    	  {
    		std_memcpy(strbuf.get(), "\0", size_t(1));
    		stmt.PutData(GetRawPtr(), SQL_NULL_DATA);
    		return;
    	  }

    	  size_t start_pos = 0; // position written up to from *perm_str_ptr
    	  size_t end_pos = 0;   // position for end of next write

    	  // For long blobs, continue to grab chunks until no more left.
    	  // Only call PutData a second time if results from the first call could
    	  // not completely fit in the buffer.
    	  while (start_pos < total_len)
    	  {
    		size_t left_to_write = total_len - start_pos;

    		// size_t len_to_copy = (left_to_write <= strbuf.size() - 1 ?
    		//	 left_to_write : strbuf.size() - 1);

    		size_t len_to_copy = (left_to_write <= strbuf.size() ?
    			 left_to_write : strbuf.size());

    		// figure what end position of blob we're copying into strbuf
    		end_pos += len_to_copy;

			// Call SQLPutData on chunk from string
    		//
			stmt.PutData(reinterpret_cast<const void *>(perm_str_ptr->data() + start_pos), 
				static_cast<SDWORD>( len_to_copy ) );

    		// get new start pos. for next iteration of while loop
    		start_pos = end_pos;
    	  }

#ifdef POSTGRES_BLOB_PATCH
//	      converted_blob.clear();			// release allocated memory.
#endif
    	}

#ifndef DTL_NO_UNICODE
    	// DLP: added to support std::wstring
    	else if (typeId == C_WSTRING)
    	{
    	  // type is wstring, cast is OK
    	  STD_::wstring *perm_str_ptr = (STD_::wstring *) addr;

    	  const size_t total_len = perm_str_ptr->length();

    	  // must handle empty strings, else ParamData() doesn't think
    	  // we're done with the parameter
    	  if (total_len == 0)
    	  {
    		std_wcsncpy(reinterpret_cast<wchar_t *>(strbuf.get()), L"\0", 1);
    		stmt.PutData(GetRawPtr(), SQL_NULL_DATA);
    		return;
    	  }

    	  size_t start_pos = 0; // position written up to from *perm_str_ptr
    	  size_t end_pos = 0;   // position for end of next write

    	  // For long wstring, continue to grab chunks until no more left.
    	  // Only call PutData a second time if results from the first call could
    	  // not completely fit in the buffer.
    	  while (start_pos < total_len)
    	  {
    		size_t left_to_write = total_len - start_pos;

    		size_t max_len_to_copy = strbuf.size()/ sizeof(wchar_t);

    		size_t len_to_copy = (left_to_write <= max_len_to_copy ?
    			 left_to_write : max_len_to_copy);

    		// figure what end position of wstring we're copying into strbuf
    		end_pos += len_to_copy;

    		// Call SQLPutData on chunk from string
    		//
            // parameter length must be changed from # of wide characters to bytes
    		// hence the multiplication of len_to_copy by sizeof(wchar_t)
    		stmt.PutData(reinterpret_cast<const void *>(perm_str_ptr->data() + start_pos), 
				static_cast<SDWORD>(len_to_copy * sizeof(wchar_t)) );

    		// get new start pos. for next iteration of while loop
    		start_pos = end_pos;
    	  }

    	}
#endif

}

// propagate results back to the bound STL strings ... done on a SELECT
// We make this non exception-safe, but that should be O.K. since BoundIOs merely
// hold temporary buffers anyway. Also, making a temporary copy for exception
// safety here will kill us performance wise.
void BoundIOs::PropagateFromResults(DBStmt &stmt)
{
    	bool bFail = false;

    	// must order columns by ColNo in order for GetData() to bind in increasing
    	// column number order
    	STD_::vector<BoundIO *> applyMoveReadOrdered(cColumns);
		STD_::vector<BoundIO *>::iterator b_it;

		// init vector to NULL
		for (b_it = applyMoveReadOrdered.begin();
    	b_it != applyMoveReadOrdered.end(); ++b_it) {
			*b_it = NULL;
		}

    	for (BoundIOs::iterator order_it = begin(); order_it != end(); ++order_it)
    	{
    		BoundIO &bio = order_it->second;
    		if (bio.IsColumn())
    		{
    			applyMoveReadOrdered[bio.GetColNo()] = &bio;
    		}
    	}

		tstring err;
    	for (b_it = applyMoveReadOrdered.begin();
    	b_it != applyMoveReadOrdered.end(); ++b_it) {

			if (*b_it == NULL)
				continue;

    		try {
    			(*b_it)->MoveRead(stmt); // individual calls exception-safe
			} catch(DBException &ex) {
    			bFail = true;
				err += ex.twhat();
    		}
    	}

    	if (bFail)
		{
			err = tstring(
				_TEXT("Call to BoundIO::MoveRead() / SQLGetData() failed.  Make sure that all string ")
				_TEXT("columns come last in your SQL statement or that your ODBC driver ")
				_TEXT("supports SQLGetData extensions. If you need support for reading columns in DTL in ")
				_TEXT("any order than you may want to define DTL_VARIANT_USE_FIXED_LEN_STRING\n\nBase error message:\n")
				) + err;
    		DTL_THROW DBException(_TEXT("BoundIOs::PropagateFromResults()"), err, NULL, NULL);
		}
}

// Resets the address of the DataObj we are working with & updates all BoundIOs to
// this new address
void BoundIOs::SetWorkingAddr(BYTE *addr, TIMESTAMP_STRUCT *pts)
{
    	working_addr = addr;
		int its = 0;
	 	for (BoundIOs::iterator bc_it = begin(); bc_it != end(); bc_it++)
    	{
    		STD_::pair<const tstring, BoundIO> &pr = *bc_it;
			if (pr.second.IsColumn() ) {
   				pr.second.SetAddr(addr + pr.second.GetOffset());
				if (pr.second.IsJtime()) {
					pr.second.SetStrbufAddr((BYTE *)(&pts[its]), sizeof(TIMESTAMP_STRUCT));
					++its;
				}
			}
    	}
}

// Resets all complex string address.  Used to MoveRead data to a particular row
void BoundIOs::SetComplexAddr(BYTE *addr, TIMESTAMP_STRUCT *pts)
{
    	working_addr = addr;
		int its = 0;
	 	for (BoundIOs::iterator bc_it = begin(); bc_it != end(); bc_it++)
    	{
    		STD_::pair<const tstring, BoundIO> &pr = *bc_it;
			if (pr.second.IsColumn() && !pr.second.IsPrimitiveColumn()) {// only reassign complex columns
    			pr.second.SetAddr(addr + pr.second.GetOffset());
				if (pr.second.IsJtime()) {
					pr.second.SetStrbufAddr((BYTE *)(&pts[its]), sizeof(TIMESTAMP_STRUCT));
					++its;
				}
			}
    	}
}

// Checks to see if list of BoundIOs has any complex string columns
bool BoundIOs::HasStrings()
{
    	for (BoundIOs::iterator bc_it = begin(); bc_it != end(); ++bc_it)
    	{
    		STD_::pair<const tstring, BoundIO> &pr = *bc_it;
    		if (pr.second.IsColumn() && pr.second.IsVariString()) // only reassign string columns
    			return true;
    	}

		return false;
}

// Checks to see if list of BoundIOs has any complex string columns
bool BoundIOs::HasCStrings()
{
    	for (BoundIOs::iterator bc_it = begin(); bc_it != end(); ++bc_it)
    	{
    		STD_::pair<const tstring, BoundIO> &pr = *bc_it;
    		if (pr.second.IsColumn() && pr.second.IsCString()) // only reassign string columns
    			return true;
    	}

		return false;
}


// set the address & initialize bound variant_row fields
void BoundIOs::BindVariantRow(variant_row &vr)
{
    	vr.init(); // get variant_row to allocate space based on field information
    	BYTE *addr = data_ptr(&vr);
    	for (BoundIOs::iterator bc_it = begin(); bc_it != end(); bc_it++)
    	{
    		STD_::pair<const tstring, BoundIO> &pr = *bc_it;
    		if (pr.second.GetAddr() == NULL) // convention in BoundIO operator==(TypeTranslationField &) is to set address to NULL for unassigned fields
    			pr.second.SetAddr(addr + pr.second.GetOffset());
    	}
}

void BoundIOs::ClearNull()
{
    	for (iterator it = begin(); it != end(); it++)
    		(*it).second.ClearNull();
}

// returns true if field name passed in null or doesn't exist
bool BoundIOs::IsNullOrNotExists(const tstring &name)
{
	return find(name) == end() || (*this)[name].IsNull();
}

// set the field for all primitive columns to NULL
void BoundIOs::InitNullFields()
{
		for (iterator it = begin(); it != end(); it++)
			(*it).second.InitNullField();
}

size_t BoundIOs_NumColumns(BoundIOs *pB) {return pB->NumColumns();}

tostream &operator<<(tostream &o, const BoundIOs &bs)
{
    for (BoundIOs::const_iterator b_it = bs.begin(); b_it != bs.end();
    			b_it++)
    {
    		o << (*b_it).second << STD_::endl;
    }
    return o;
}

// facilities used by LocalBCA
// operator that generates a BoundIOs structure from two raw BoundIO's
BoundIOs BoundIO::operator&&(const BoundIO &addMe)
{
    BoundIO orig(*this);
    BoundIO add(addMe);

    BoundIOs newParentBoundIOs;

    BoundIO &first = newParentBoundIOs[GetName()] = orig;
    BoundIO &second = newParentBoundIOs[addMe.GetName()] = add;

    // update parent BoundIO pointers
    first.pBoundIOs = &newParentBoundIOs;
    second.pBoundIOs = &newParentBoundIOs;

    if (IsColumn())
    {
    	first.colNo = 0;
    	second.colNo = 1;
    }

    return newParentBoundIOs;
}

TypeTranslation BoundIO::GetTypeTranslation() const
{
    ETI_Map &SQL_types_to_C = GetSQL_types_to_C();

    ETI_Map::iterator it = SQL_types_to_C.find_by_typeid(typeId);

    if (it == SQL_types_to_C.end())
    	DTL_THROW ETIException(_TEXT("BoundIO::GetTypeTranslation()"),
    		_TEXT("Invalid typeId in BoundIO!"));

    return it->second;
}

// facilities used by LocalBCA
// operator that generates a BoundIOs structure from an
// already existing BoundIOs and a raw BoundIO
BoundIOs BoundIOs::operator&&(const BoundIO &addMe)
{
    BoundIO add(addMe);

    BoundIOs newParentBoundIOs(*this);

    BoundIO &second = newParentBoundIOs[addMe.GetName()] = add;

    // update parent BoundIO pointers
    second.pBoundIOs = &newParentBoundIOs;

    if (second.IsColumn())
    {
    	second.colNo = static_cast<SDWORD>( BoundIOs_NumColumns(&newParentBoundIOs) - 1);
    }

    return newParentBoundIOs;
}

void BoundIO::SetBoundIOsPtr(BoundIOs *ptr)
{
    pBoundIOs = ptr;
}

END_DTL_NAMESPACE
