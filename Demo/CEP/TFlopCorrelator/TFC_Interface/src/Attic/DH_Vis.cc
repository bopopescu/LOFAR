//  DH_Vis.cc:
//
//  Copyright (C) 2004
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//
//
//  $Id$
//
//
//////////////////////////////////////////////////////////////////////
#include <lofar_config.h>

#include <APS/ParameterSet.h>

#include <DH_Vis.h>

namespace LOFAR
{

DH_Vis::DH_Vis (const string& name, double centerFreq, 
		const ACC::APS::ParameterSet pSet)
: DataHolder    (name, "DH_Vis"),
  itsPS         (pSet),
  itsBuffer     (0),
  itsCenterFreq  (centerFreq) 
{
   itsNPols = itsPS.getInt32("Input.NPolarisations");
   itsNCorrs = itsNPols*itsNPols;
   itsNStations  = itsPS.getInt32("Input.NRSP");
   itsNBaselines = itsNCorrs * itsNStations * (itsNStations + 1)/2;
   
   itsNsamples = itsPS.getInt32("WH_Corr.samples");
}

DH_Vis::DH_Vis(const DH_Vis& that)
  : DataHolder    (that),
    itsPS         (that.itsPS),
    itsBuffer     (0),
    itsCenterFreq (that.itsCenterFreq),
    itsNStations  (that.itsNStations),
    itsNBaselines (that.itsNBaselines),
    itsNPols      (that.itsNPols),
    itsNCorrs     (that.itsNCorrs)
{}

DH_Vis::~DH_Vis()
{}

DataHolder* DH_Vis::clone() const
{
  return new DH_Vis(*this);
}

void DH_Vis::init()
{
  // Determine the size of the buffer.
  itsBufSize = itsNBaselines;
  addField("Buffer", BlobField<BufferType>(1, itsBufSize));
  createDataBlock();  // calls fillDataPointers

  memset(itsBuffer, 0, itsBufSize*sizeof(BufferType)); 
}

void DH_Vis::fillDataPointers() 
{
  itsBuffer = getData<BufferType> ("Buffer");
}


}
