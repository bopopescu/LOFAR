//#  BWWrite.cc: implementation of the BWWrite class
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

#include <APL/RSP_Protocol/EPA_Protocol.ph>
#include <APL/RSP_Protocol/RSP_Protocol.ph>

#include <APL/RTCCommon/PSAccess.h>

#include <unistd.h>
#include <string.h>
#include <blitz/array.h>

#include "BWWrite.h"
#include "Cache.h"
#include "StationSettings.h"

#define N_RETRIES 3

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace EPA_Protocol;

BWWrite::BWWrite(GCFPortInterface& board_port, int board_id, int blp, int regid)
  : SyncAction(board_port, board_id, BF_N_FRAGMENTS),
    m_blp(blp), m_regid(regid), m_remaining(0), m_offset(0)
{
  memset(&m_hdr, 0, sizeof(MEPHeader));
}

BWWrite::~BWWrite()
{
}

void BWWrite::sendrequest()
{
  uint8 global_blp = (getBoardId() * StationSettings::instance()->nrBlpsPerBoard()) + m_blp;

  // no conditional, update every second

  // reset m_offset and m_remaining for each register
  if (0 == getCurrentIndex()) {
    m_offset = MEPHeader::N_LOCAL_XLETS * MEPHeader::WEIGHT_SIZE;
    m_remaining = MEPHeader::BF_XROUT_SIZE - m_offset; // representative for XR, XI, YR, YI size
  }

  if (m_regid < MEPHeader::BF_XROUT || m_regid > MEPHeader::BF_YIOUT)
  {
    LOG_FATAL("invalid regid");
    exit(EXIT_FAILURE);
  }

  LOG_DEBUG(formatString(">>>> BWWrite(%s) global_blp=%d, blp=%d, regid=%d, m_offset=%d, m_remaining=%d",
			 getBoardPort().getName().c_str(),
			 global_blp, m_blp, m_regid, m_offset, m_remaining));
  
  // send next BF configure message
  EPABfCoefsWriteEvent bfcoefs;

  size_t size = (MEPHeader::N_BEAMLETS * MEPHeader::WEIGHT_SIZE) / BF_N_FRAGMENTS;
  LOG_DEBUG_STR("size=" << size);

  // this code is only guaranteed to work under the following conditions
  ASSERT(size < MEPHeader::FRAGMENT_SIZE);

  switch (m_regid)
  {
    case MEPHeader::BF_XROUT:
      bfcoefs.hdr.set(MEPHeader::BF_XROUT_HDR, 1 << m_blp,
		      MEPHeader::WRITE, size, m_offset);
      break;
    case MEPHeader::BF_XIOUT:
      bfcoefs.hdr.set(MEPHeader::BF_XIOUT_HDR, 1 << m_blp,
		      MEPHeader::WRITE, size, m_offset);
      break;
    case MEPHeader::BF_YROUT:
      bfcoefs.hdr.set(MEPHeader::BF_YROUT_HDR, 1 << m_blp,
		      MEPHeader::WRITE, size, m_offset);
      break;
    case MEPHeader::BF_YIOUT:
      bfcoefs.hdr.set(MEPHeader::BF_YIOUT_HDR, 1 << m_blp,
		      MEPHeader::WRITE, size, m_offset);
      break;
  }

  // create blitz view om the weights in the bfcoefs message to be sent to the RSP hardware
  int nbeamlets_per_fragment = MEPHeader::N_BEAMLETS / BF_N_FRAGMENTS;
  Array<complex<int16>, 2> weights(nbeamlets_per_fragment, MEPHeader::N_POL);
  bfcoefs.coef.setBuffer(weights.data(), weights.size() * sizeof(complex<uint16>));

  LOG_DEBUG_STR("weights shape=" << weights.shape());

  ASSERT(MEPHeader::N_BEAMLETS % BF_N_FRAGMENTS == 0);
  for (int lane = 0; lane < MEPHeader::N_SERDES_LANES; lane++) {

    int hw_offset = lane;
    int cache_offset = lane * (MEPHeader::N_BEAMLETS / MEPHeader::N_SERDES_LANES) + (getCurrentIndex() * nbeamlets_per_fragment / MEPHeader::N_SERDES_LANES);

    Range hw_range(hw_offset, hw_offset + nbeamlets_per_fragment - MEPHeader::N_BLPS, MEPHeader::N_BLPS);
    Range cache_range(cache_offset, cache_offset + (nbeamlets_per_fragment / MEPHeader::N_SERDES_LANES) - 1, 1);
    
    LOG_DEBUG_STR("lane=" << lane);
    LOG_DEBUG_STR("hw_range=" << hw_range);
    LOG_DEBUG_STR("cache_range=" << cache_range);

    // X
    weights(hw_range, 0) = Cache::getInstance().getBack().getBeamletWeights()()(0, global_blp * 2, cache_range);

    // Y
    weights(hw_range, 1) = Cache::getInstance().getBack().getBeamletWeights()()(0, global_blp * 2 + 1, cache_range);
  }

  // update m_remaining and m_offset for next write
  m_remaining -= size;
  m_offset    += size;

  //int xc_gain = GET_CONFIG("RSPDriver.XC_GAIN", i);

  switch (m_regid)
  {
    case MEPHeader::BF_XROUT:
    {
      // weights for x-real part
      // no added conversions needed

      // y weights should be 0
      weights(Range::all(), 1) = 0;
    }
    break;

    case MEPHeader::BF_XIOUT:
    {
      // weights for x-imaginary part
      weights *= complex<int16>(0,1);

      // y weights should be 0
      weights(Range::all(), 1) = 0;
    }
    break;
    
    case MEPHeader::BF_YROUT:
    {
      // weights for y-real part
      // no added conversions needed

      // x weights should be 0
      weights(Range::all(), 0) = 0;
    }
    break;
    
    case MEPHeader::BF_YIOUT:
    {
      // weights for y-imaginary part
      weights *= complex<int16>(0,1);

      // x weights should be 0
      weights(Range::all(), 0) = 0;
    }
    break;
  }

  m_hdr = bfcoefs.hdr;
  getBoardPort().send(bfcoefs);
}

void BWWrite::sendrequest_status()
{
  // intentionally left empty
}

GCFEvent::TResult BWWrite::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  if (EPA_WRITEACK != event.signal)
  {
    LOG_WARN("BWWrite::handleack: unexpected ack");
    return GCFEvent::NOT_HANDLED;
  }
  
  EPAWriteackEvent ack(event);

  uint8 global_blp = (getBoardId() * StationSettings::instance()->nrBlpsPerBoard()) + m_blp;

  if (!ack.hdr.isValidAck(m_hdr))
  {
    Cache::getInstance().getState().bf().write_error(global_blp * MEPHeader::N_PHASEPOL + m_regid);

    LOG_ERROR("BWWrite::handleack: invalid ack");
    return GCFEvent::NOT_HANDLED;

  } else {

    //
    // Last fragment signals completion
    //
    if (BF_N_FRAGMENTS - 1 == getCurrentIndex()) {
      Cache::getInstance().getState().bf().write_ack(global_blp * MEPHeader::N_PHASEPOL + m_regid);
    }

  }

  return GCFEvent::HANDLED;
}


