//#  Command.cc: implementation of the Command class
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

#include "Command.h"

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

using namespace RSP;
using namespace LOFAR;

Command::Command() : m_period(0), m_event(0), m_answerport(0), m_operation(READ)
{
}

Command::~Command()
{
  /* TODO: delete event? */
}

void Command::setPeriod(int16 period)
{
  m_period = period;
}

void Command::setEvent(GCFEvent* event)
{
  /* TODO: delete preexisting event? */

  m_event = event;
}

void Command::setAnswerPort(GCFPortInterface* port)
{
  m_answerport = port;
}

void Command::apply()
{
}
