//#  AVTPropertySetAnswer.cc: Implementation of the PropertySet Answer
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

#include <Common/LofarLogger.h>
#include <APLCommon/APL_Defines.h>
#include "AVTPropertySetAnswer.h"
#include "AVTPropertySetAnswerHandlerInterface.h"

using namespace LOFAR;
using namespace LOFAR::AVT;
using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PAL;

INIT_TRACER_CONTEXT(AVTPropertySetAnswer,LOFARLOGGER_PACKAGE);

AVTPropertySetAnswer::AVTPropertySetAnswer(AVTPropertySetAnswerHandlerInterface& handler) :
  GCFAnswer(),
  m_handler(handler)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,"propertySetAnswer");
}

AVTPropertySetAnswer::~AVTPropertySetAnswer()
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,"propertySetAnswer");
}

void AVTPropertySetAnswer::handleAnswer(GCFEvent& answer)
{
  m_handler.handlePropertySetAnswer(answer);
}
