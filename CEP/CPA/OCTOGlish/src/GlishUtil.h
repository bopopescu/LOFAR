#ifndef OCTOGlish_GlishUtil_h
#define OCTOGlish_GlishUtil_h 1

#include <DMI/DMI.h>
#include <DMI/DataRecord.h>
#include <DMI/DataField.h>
#include <DMI/DataArray.h>
    
#include <aips/Glish.h>

namespace GlishUtil
{
    
  GlishArray makeFailField ( const String &msg );

  bool makeGlishArray (GlishArray &arr,const NestableContainer &nc,TypeId tid,bool adjustIndex);
    
  GlishRecord recToGlish (const DataRecord &rec);

  GlishValue objectToGlishValue (const BlockableObject &obj,bool adjustIndex);

  ObjRef makeDataArray (const GlishArray &arr,bool adjustIndex);

  ObjRef makeDataField (const GlishArray &arr,bool adjustIndex);

  ObjRef glishValueToObject (const GlishValue &val,bool adjustIndex);

  GlishRecord objectToBlockRec (const BlockableObject &obj);

  BlockableObject * blockRecToObject (const GlishRecord &rec);

  template<class T> 
  void newDataField (ObjRef &ref,const GlishArray &arr);
  
  template<class T> 
  void newDataArray (ObjRef &ref,const GlishArray &arr);
}
#endif
