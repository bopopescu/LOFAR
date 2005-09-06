//#  Versions.h: implementation of the Versions class
//#
//#  Copyright (C) 2002-2004
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
#include <Common/LofarLogger.h>

#include "Versions.h"
#include "Marshalling.h"

using namespace LOFAR;
using namespace RSP_Protocol;
using namespace std;
using namespace blitz;

unsigned int Versions::getSize()
{
  return
      MSH_ARRAY_SIZE(m_rsp_versions, uint8)
    + MSH_ARRAY_SIZE(m_bp_versions, uint8)
    + MSH_ARRAY_SIZE(m_ap_versions, uint8);
}

unsigned int Versions::pack  (void* buffer)
{
  unsigned int offset = 0;

  MSH_PACK_ARRAY(buffer, offset, m_rsp_versions, uint8);
  MSH_PACK_ARRAY(buffer, offset, m_bp_versions,  uint8);
  MSH_PACK_ARRAY(buffer, offset, m_ap_versions,  uint8);

  return offset;
}

unsigned int Versions::unpack(void *buffer)
{
  unsigned int offset = 0;

  MSH_UNPACK_ARRAY(buffer, offset, m_rsp_versions, uint8, 1);
  MSH_UNPACK_ARRAY(buffer, offset, m_bp_versions,  uint8, 1);
  MSH_UNPACK_ARRAY(buffer, offset, m_ap_versions,  uint8, 1);

  return offset;
}
