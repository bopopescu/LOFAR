//#  -*- mode: c++ -*-
//#
//#  SSSync.h: Synchronize subbands selection settings with RSP hardware.
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

#ifndef SSSYNC_H_
#define SSSYNC_H_

#include "SyncAction.h"
#include <Common/LofarTypes.h>

namespace RSP
{
  class SSSync : public SyncAction
  {
    public:
      /**
       * Constructors for a SSSync object.
       */
      SSSync(GCFPortInterface& board_port, int board_id);
	  
      /* Destructor for SSSync. */
      virtual ~SSSync();

      /**
       * Write subband selection info.
       */
      virtual void sendrequest();

      /**
       * Read the board status.
       */
      virtual void sendrequest_status();

      /**
       * Handle the READRES message.
       */
      virtual GCFEvent::TResult handleack(GCFEvent& event, GCFPortInterface& port);
  };
};
     
#endif /* SSSYNC_H_ */
