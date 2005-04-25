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


#include <DH_Vis.h>
#include <Common/KeyValueMap.h>

namespace LOFAR
{

DH_Vis::DH_Vis (const string& name)
: DataHolder    (name, "DH_Vis"),
  itsBuffer     (0),
  itsNPol       (2)
{
  ParameterSet  myPS("TFlopCorrelator.cfg");
  //ParameterCollection	myPC(myPS);
  itsNStations  = myPS("DH_CorrCube.stations");
}

DH_Vis::DH_Vis(const DH_Vis& that)
  : DataHolder(that),
    itsBuffer(0)
{
  itsNPol = that.itsNPol;
  itsNStations = that.itsNStations;
}

DH_Vis::~DH_Vis()
{
}

DataHolder* DH_Vis::clone() const
{
  return new DH_Vis(*this);
}

void DH_Vis::preprocess()
{
  // First delete possible buffers.
  postprocess();

  // Determine the number of bytes needed for DataPacket and buffer.
  itsBufSize = nstations * nstations * nchannels * npolarisations*npolarisations; 

  addField("Buffer", BlobField<BufferType>(1, itsBufSize));
  createDataBlock();  // calls fillDataPointers
  //itsBuffer = getData<BufferType> ("Buffer");
  // todo: memset instead of loop
  for (unsigned int i=0; i<itsBufSize; i++) {
    itsBuffer[i] = 0;
  }
}

void DH_Vis::postprocess()
{
  itsBuffer     = 0;
}

void DH_Vis::fillDataPointers() {
  itsBuffer = getData<BufferType> ("Buffer");

}


}
