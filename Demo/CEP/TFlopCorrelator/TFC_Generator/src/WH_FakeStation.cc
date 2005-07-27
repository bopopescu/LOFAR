//#  WH_FakeStation.cc: Emulate a station
//#
//#  Copyright (C) 2002-2005
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#include <lofar_config.h>

// General includes
#include <Common/LofarLogger.h>

// Application specific includes
#include <TFC_Interface/RSPTimeStamp.h>
#include <DH_RSP.h>
#include <WH_FakeStation.h>

using namespace LOFAR;

WH_FakeStation::WH_FakeStation(const string& name, 
			       const ParameterSet ps,
			       TransportHolder& th,
			       const int stationID,
			       const int delay)
  : WorkHolder (1, 0,
		name, 
		"WH_FakeStation"),
    itsPS(ps),
    itsTH(th),
    itsDelay(delay)
{
  itsStationId = stationID;
  getDataManager().addInDataHolder(0, new DH_RSP("incoming_DH_RSP", itsPS));
}

WH_FakeStation::~WH_FakeStation() {
}

WorkHolder* WH_FakeStation::construct(const string& name,
				      const ParameterSet ps,
				      TransportHolder& th,
				      const int stationID,
				      const int delay)
{
  return new WH_FakeStation(name, ps, th, stationID, delay);
}

WH_FakeStation* WH_FakeStation::make(const string& name)
{
  return new WH_FakeStation(name, itsPS, itsTH, itsStationId, itsDelay);
}

void WH_FakeStation::process() 
{
  EthernetFrame& myEthFrame = ((DH_RSP*)getDataManager().getInHolder(0))->getEthernetFrame();
  TimeStamp myStamp;

  for (int epap = 0; epap < myEthFrame.getNoPacketsInFrame(); epap++) {
    EpaHeader& header = myEthFrame.getEpaPacket(epap).getHeader();

    // set my station id
    header.setStationId(itsStationId);

    // get the stamp
    myStamp.setStamp(header.getSeqId(), header.getBlockId());
    // add the delay
    myStamp += itsDelay;
    // set the stamp
    header.setSeqId(myStamp.getSeqId());
    header.setBlockId(myStamp.getBlockId());
  }
  bool ret = itsTH.sendBlocking(myEthFrame.getPayloadp(), myEthFrame.getPayloadSize(), 0);
  ASSERTSTR(ret, "TH couldn't send data");
}
