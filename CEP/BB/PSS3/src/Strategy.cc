//#  Strategy.cc:  A base class for all calibration strategies
//#
//#  Copyright (C) 2002-2003
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

#include <PSS3/Strategy.h>
#include <Common/Debug.h>
#include <PSS3/SI_Peeling.h>

Strategy::Strategy(int strategyNo, Calibrator* cal, 
		   int varArgSize, char* varArgs)
{
  AssertStr(cal!=0, "Calibrator pointer is 0");
  switch (strategyNo) 
  {
  case 1:                                        // Peeling
    TRACER3("Creating peeling strategyImpl");
    itsImpl = new SI_Peeling(cal, varArgSize, varArgs);
    break;
  case 2:                                        // Ripple Tree
    TRACER3("Creating ripple tree strategyImpl");
    // Code must be added
    break;
  case 3:                                        // Classic
    TRACER3("Creating classic strategyImpl");
    // Code must be added
    break;
  default:
    itsImpl = 0;
    Throw("Unknown strategy number in Strategy construction");
  }

}

Strategy::~Strategy()
{
  if (itsImpl != 0)
  {
    delete itsImpl;
  }
}

bool Strategy::execute(vector<string>& parmNames,                 
		       vector<float>& parmValues,
		       Solution& solutionQuality,
		       int& source)
{
  if (itsImpl != 0)
  {
    return itsImpl->execute(parmNames, parmValues, solutionQuality, source);
  }
  else
  {
    TRACER2("No strategy implementation; cannot execute.");
    return false;
  }
}
