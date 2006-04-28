//#  TDSResultRead.cc: implementation of the TDSResultRead class
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

#include <APL/RTCCommon/PSAccess.h>
#include <string.h>

#include "TDSResultRead.h"
#include "TDSi2cdefs.h"
#include "Cache.h"

#include <netinet/in.h>

using namespace LOFAR;
using namespace RSP;
using namespace EPA_Protocol;

#define TDSRESULTREAD_DELAY 2

TDSResultRead::TDSResultRead(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, 1), m_delay(0)
{
  memset(&m_hdr, 0, sizeof(MEPHeader));
}

TDSResultRead::~TDSResultRead()
{
  /* TODO: delete event? */
}

void TDSResultRead::sendrequest()
{
  // skip update if the Clocks settings have not been modified
  if (RTC::RegisterState::APPLIED != Cache::getInstance().getTDSState().get(getBoardId()))
  {
    setContinue(true);
    return;
  }

  // delay TDSRESULTREAD_DELAY periods
  if (m_delay++ < TDSRESULTREAD_DELAY) {
    setContinue(true);
    return;
  }
  m_delay = 0;

  EPAReadEvent tdsresult;
  tdsresult.hdr.set(MEPHeader::TDS_RESULT_HDR);

  m_hdr = tdsresult.hdr; // remember header to match with ack
  getBoardPort().send(tdsresult);
}

void TDSResultRead::sendrequest_status()
{
  // intentionally left empty
}

//
// Compare n bytes of the two buffers.
// Return -1 when buffers are identical
// Return index of first difference, when buffers are different
//
static int imemcmp(const void* buf1, const void* buf2, int n)
{
  const char* b1 = (char*)buf1;
  const char* b2 = (char*)buf2;

  register int i = 0;
  for (i = 0; i < n; i++) if (b1[i] != b2[i]) break;
  
  return (i != n ? i : -1);
}

#if 0
void printbin(void* buf, int n)
{
  unsigned char* c = (unsigned char*)buf;
  for (int i = 0; i< n; i++) {
    if (0 == i % 3) printf("\n");
    printf("%02x ", c[i]);
  }
  printf("\n");
}
#else
#define printbin(buf, n) do { } while (0)
#endif

GCFEvent::TResult TDSResultRead::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  if (EPA_TDS_RESULT != event.signal)
    {
      LOG_WARN("TDSResultRead::handleack:: unexpected ack");
      return GCFEvent::NOT_HANDLED;
    }
  
  EPATdsResultEvent ack(event);

  if (!ack.hdr.isValidAck(m_hdr))
    {
      LOG_ERROR("TDSResultRead::handleack: invalid ack");
      return GCFEvent::NOT_HANDLED;
    }

  int idiff = 0;

  switch (Cache::getInstance().getBack().getClock()) {

  case 160:
    printbin(tds_160MHz_result, sizeof(tds_160MHz_result));
    printbin(ack.result, sizeof(tds_160MHz_result));
    idiff = imemcmp(tds_160MHz_result, ack.result, sizeof(tds_160MHz_result));
    if (-1 == idiff) {
      Cache::getInstance().getTDSState().confirmed(getBoardId());
    } else {
      LOG_ERROR(formatString("TDSResultRead::handleack (160MHz): unexpected I2C result response, first mismatch @ %d", idiff));
    }
    break;

  case 200:
    printbin(tds_200MHz_result, sizeof(tds_200MHz_result));
    printbin(ack.result, sizeof(tds_200MHz_result));
    idiff = imemcmp(tds_200MHz_result, ack.result, sizeof(tds_200MHz_result));
    if (-1 == idiff) {
      Cache::getInstance().getTDSState().confirmed(getBoardId());
    } else {
      LOG_ERROR(formatString("TDSResultRead::handleack (200MHz): unexpected I2C result response, first mismatch @ %d", idiff));
    }
    break;

  default:
    printbin(tds_off_result, sizeof(tds_off_result));
    printbin(ack.result, sizeof(tds_off_result));
    idiff = imemcmp(tds_off_result, ack.result, sizeof(tds_off_result));
    if (-1 == idiff) {
      Cache::getInstance().getTDSState().confirmed(getBoardId());
    } else {
      LOG_ERROR(formatString("TDSResultRead::handleack (OFF): unexpected I2C result response, first mismatch @ %d", idiff));
    }
    break;
  }

  return GCFEvent::HANDLED;
}
