//# BBSRefitStep.h: Derived leaf class of the BBSStep composite pattern.
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

#ifndef LOFAR_BBSCONTROL_BBSREFITSTEP_H
#define LOFAR_BBSCONTROL_BBSREFITSTEP_H

// \file
// Derived leaf class of the BBSStep composite pattern.

//# Includes
#include <BBSControl/BBSSingleStep.h>

namespace LOFAR
{
  namespace BBS
  {
    // \addtogroup BBSControl
    // @{

    // This is a so-called \e leaf class in the BBSStep composite pattern (see
    // Gamma, 1995).
    // \note Currently, a %BBSRefitStep is in fact identical to a
    // BBSSingleStep. Only the classType() method is overridden.
    class BBSRefitStep : public BBSSingleStep
    {
    public:
      BBSRefitStep(const BBSStep* parent = 0) : 
        BBSSingleStep(parent)
      {
      }

      BBSRefitStep(const string& name, 
		   const ACC::APS::ParameterSet& parSet,
		   const BBSStep* parent) :
        BBSSingleStep(name, parSet, parent)
      {
      }

      // Accept a CommandVisitor that wants to process \c *this.
      virtual void accept(CommandVisitor &visitor) const;

      // Return the operation type of \c *this as a string.
      virtual const string& operation() const;

      // Return the command type of \c *this as a string.
      virtual const string& type() const;
    };

    // @}
    
  } // namespace BBS

} // namespace LOFAR

#endif
