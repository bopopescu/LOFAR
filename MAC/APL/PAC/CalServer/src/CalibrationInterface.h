//#  -*- mode: c++ -*-
//#  CalibrationInterface.h: class definition for the Beam Server task.
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

#ifndef CALIBRATIONINTERFACE_H_
#define CALIBRATIONINTERFACE_H_

#include "SubArray.h"
#include "ACC.h"
#include "CalibrationResult.h"

namespace CAL
{
  class CalibrationInterface
  {
  public:
    //CalibrationInterface() = 0;
    virtual ~CalibrationInterface() {}
      
    /**
     * Calibrate the specified subarray. Store the result in the CalibrationResult object.
     * @param subarray The subarray to calibrate. Use SubArray methods to get relevant parameters.
     * @param result The calibration result should be stored in this object.
     */
    virtual void calibrate(const SubArray& subarray, const ACC& acc, CalibrationResult& result) = 0;
  };
};

#endif /* CALIBRATIONINTERFACE_H_ */

