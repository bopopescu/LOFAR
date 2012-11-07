//#  RSP: RSP data format
//#
//#  Copyright (C) 2008
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

#ifndef LOFAR_IONPROC_RSP_H
#define LOFAR_IONPROC_RSP_H

#include <Common/LofarTypes.h>

namespace LOFAR {
namespace RTCP {

#include <cstddef>


// All data is in Little Endian format!

struct RSP {
  struct Header {
    uint8  version;
    uint8  sourceInfo;
    uint16 configuration;
    uint16 station;
    uint8  nrBeamlets;
    uint8  nrBlocks;
    uint32 timestamp;
    uint32 blockSequenceNumber;
  } header;

  char       data[8130];
};

} // namespace RTCP
} // namespace LOFAR

#endif