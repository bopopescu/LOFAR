//# BBSSingleStep.h: Derived leaf class of the BBSStep composite pattern.
//#
//# Copyright (C) 2006
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#ifndef LOFAR_BBSCONTROL_BBSSINGLESTEP_H
#define LOFAR_BBSCONTROL_BBSSINGLESTEP_H

// \file
// Derived leaf class of the BBSStep composite pattern.

//# Includes
#include <BBSControl/BBSStep.h>

namespace LOFAR
{
  namespace BBS
  {
    // \addtogroup BBS
    // @{

    // This is a so-called \e leaf class in the BBSStep composite pattern (see
    // Gamma, 1995).
    // \note %BBSSingleStep not implemented as a leaf class; it contains a
    // number of data members that are common to "real" BBSStep leaf classes,
    // like BBSSolveStep.
    class BBSSingleStep : public BBSStep
    {
    public:
      // Construct a BBSSingleStep having the name \a name. Configuration
      // information for this step can be retrieved from the parameter set \a
      // parset, by searching for keys <tt>Step.\a name</tt>. \a parent
      // is a pointer to the BBSStep object that is the parent of \c *this.
      BBSSingleStep(const string& name,
		    const ACC::APS::ParameterSet& parset,
		    const BBSStep* parent);

      virtual ~BBSSingleStep();

      // Print the contents of \c *this in human readable form.
      virtual void print(ostream& os) const;

      virtual void execute(const StrategyController*) const;

    private:
      // Name of the data column to write data to
      string          itsOutputData;

    };

    // For the time being we'll define the following steps as typedefs. If,
    // and when, they need to be "upgraded" to a real class, we can do so,
    // without affecting code referring to these types.
    // @{
    typedef BBSSingleStep BBSSubtractStep;
    typedef BBSSingleStep BBSCorrectStep;
    typedef BBSSingleStep BBSPredictStep;
    typedef BBSSingleStep BBSShiftStep;
    typedef BBSSingleStep BBSRefitStep;
    // @}

    // @}
    
  } // namespace BBS

} // namespace LOFAR

#endif
