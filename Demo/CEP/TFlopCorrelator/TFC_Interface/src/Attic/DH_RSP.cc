//#  DH_RSP.cc: DataHolder storing RSP raw ethernet frames for 
//#             StationCorrelator demo
//#
//#  Copyright (C) 2000, 2001
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#
//#  $Id$
//#
///////////////////////////////////////////////////////////////////

#include <DH_RSP.h>
#include <complex>

using std::complex;


namespace LOFAR
{

DH_RSP::DH_RSP (const string& name,
                const ACC::ParameterSet pset)
: DataHolder (name, "DH_RSP"),
  itsBuffer  (0),
  itsPSet    (pset)
{

   itsEPAheaderSize   = pset.getInt("SzEPAheader");
   itsNoBeamlets      = pset.getInt("NoRSPBeamlets");
   itsNoPolarisations = pset.getInt("polarisations");
   const int NoPacketsInFrame = pset.getInt("NoPacketsInFrame");
   itsBufSize         =  NoPacketsInFrame * 
     (itsEPAheaderSize + itsNoBeamlets * itsNoPolarisations * sizeof(complex<int16>));
}

DH_RSP::DH_RSP(const DH_RSP& that)
: DataHolder         (that),
  itsBuffer          (0),
  itsEPAheaderSize   (that.itsEPAheaderSize),
  itsNoBeamlets      (that.itsNoBeamlets),
  itsNoPolarisations (that.itsNoPolarisations),
  itsBufSize         (that.itsBufSize),
  itsPSet            (that.itsPSet)
{}

DH_RSP::~DH_RSP()
{}

DataHolder* DH_RSP::clone() const
{
  return new DH_RSP(*this);
}

void DH_RSP::init()
{
  // Add the fields to the data definition.
  addField ("Buffer", BlobField<BufferType>(1,itsBufSize));
  addField ("Flag", BlobField<int>(1));
  addField ("SyncedStamp", BlobField<char>(sizeof(timestamp_t)));
  // Create the data blob
  createDataBlock();
}

void DH_RSP::fillDataPointers()
{
  // Fill in the buffer pointer.
  itsBuffer  = getData<BufferType> ("Buffer");
  // Fill in the flag pointer.
  itsFlagPtr = getData<int> ("Flag");
  // Fill in Synchronized timestamp
  itsSyncedStampPtr = (timestamp_t*)getData<char> ("SyncedStamp");

  // use memset to null the buffer instead of a for loop
  memset(itsBuffer, 0, itsBufSize*sizeof(BufferType));
}


} // end namespace
