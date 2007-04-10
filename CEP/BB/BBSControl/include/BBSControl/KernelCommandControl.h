//# KernelCommandControl.h: 
//#
//# Copyright (C) 2007
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

#ifndef LOFAR_BBSCONTROL_KERNELCOMMANDCONTROL_H
#define LOFAR_BBSCONTROL_KERNELCOMMANDCONTROL_H

#include <BBSControl/CommandHandler.h>
#include <BBSControl/BlobStreamableConnection.h>
//#include <BBSControl/BBSStructs.h>

#include <BBSKernel/Prediffer.h>

#include <Common/lofar_smartptr.h>


namespace LOFAR
{
//# Forward declations
class BlobStreamable;

namespace BBS
{
//# Forward declations
class CommandQueue;
class Prediffer;
class RegionOfInterest;


class KernelCommandControl: public CommandHandler
{
public:
    KernelCommandControl(shared_ptr<CommandQueue> &queue,
        shared_ptr<BlobStreamableConnection> &solver)
        :   itsCommandQueue(queue),
            itsSolverConnection(solver)
    {
    }

    virtual ~KernelCommandControl()
    {
    }

    // @name Implementation of handle() for different commands.
    // @{
    virtual void handle(const InitializeCommand &command);
    virtual void handle(const FinalizeCommand &command);
    virtual void handle(const NextChunkCommand &command);
    virtual void handle(const BBSStrategy &command);
    virtual void handle(const BBSStep &command);
    virtual void handle(const BBSMultiStep &command);
    virtual void handle(const BBSSingleStep &command);
    virtual void handle(const BBSPredictStep &command);
    virtual void handle(const BBSSubtractStep &command);
    virtual void handle(const BBSCorrectStep &command);
    virtual void handle(const BBSSolveStep &command);
    virtual void handle(const BBSShiftStep &command);
    virtual void handle(const BBSRefitStep &command);
    // @}

private:
    bool convertTime(string in, double &out);
    
    // Kernel.
    scoped_ptr<Prediffer>                   itsKernel;

    // CommandQueue.
    shared_ptr<CommandQueue>                itsCommandQueue;

    // Connection to the solver.
    shared_ptr<BlobStreamableConnection>    itsSolverConnection;
    
    // Region of interest.
    int32                                   itsROIFrequency[2];
    double                                  itsROITime[2];

    // Chunk size and position (in time).
    double                                  itsChunkSize;
    double                                  itsChunkPosition;
};

} //# namespace BBS
} //# namespace LOFAR

#endif
