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
      // Default constructor. Construct an empty BBSSingleStep object and make
      // it a child of the BBSStep object \a parent.
      BBSSingleStep(const BBSStep* parent = 0);

      // Construct a BBSSingleStep having the name \a name. Configuration
      // information for this step can be retrieved from the parameter set \a
      // parset, by searching for keys <tt>Step.\a name</tt>. \a parent
      // is a pointer to the BBSStep object that is the parent of \c *this.
      BBSSingleStep(const string& name,
		    const ACC::APS::ParameterSet& parset,
		    const BBSStep* parent);

      virtual ~BBSSingleStep();

      // Print the contents of \c *this in human readable form into the output
      // stream \a os.
      virtual void print(ostream& os) const;

    protected:
      // Write the contents of \c *this into the blob output stream \a bos.
      virtual void write(BlobOStream& bos) const;

      // Read the contents from the blob input stream \a bis into \c *this.
      virtual void read(BlobIStream& bis);

    public:
      // Return the type of \c *this as a string.
      virtual const string& type() const;

      // Name of the data column to write data to
      string          itsOutputData;
    };

    
    class BBSPredictStep: public BBSSingleStep
    {
    public:
        BBSPredictStep(const BBSStep* parent = 0)
        : BBSSingleStep(parent)
        {
        }
        
        BBSPredictStep(const string& name, 
            const ACC::APS::ParameterSet& parSet,
            const BBSStep* parent)
        :BBSSingleStep(name, parSet, parent)
        {
        }
    };
    
    
    class BBSSubtractStep: public BBSSingleStep
    {
    public:
        BBSSubtractStep(const BBSStep* parent = 0)
        : BBSSingleStep(parent)
        {
        }
    
        BBSSubtractStep(const string& name, 
            const ACC::APS::ParameterSet& parSet,
            const BBSStep* parent)
        : BBSSingleStep(name, parSet, parent)
        {
        }
    };
    
    
    class BBSCorrectStep: public BBSSingleStep
    {
    public:
        BBSCorrectStep(const BBSStep* parent = 0)
        : BBSSingleStep(parent)
        {
        }
    
        BBSCorrectStep(const string& name, 
            const ACC::APS::ParameterSet& parSet,
            const BBSStep* parent)
        : BBSSingleStep(name, parSet, parent)
        {
        }
    };
    
    
    // For the time being we'll define the following steps as typedefs. If,
    // and when, they need to be "upgraded" to a real class, we can do so,
    // without affecting code referring to these types.
    // @{
    typedef BBSSingleStep BBSShiftStep;
    typedef BBSSingleStep BBSRefitStep;
    // @}

    // @}
    
  } // namespace BBS

} // namespace LOFAR

#endif
