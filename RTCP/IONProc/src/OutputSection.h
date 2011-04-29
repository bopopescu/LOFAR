//#  OldOutputSection.h: Collects data from CNs and sends data to Storage
//#
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

#ifndef LOFAR_IONPROC_OUTPUT_SECTION_H
#define LOFAR_IONPROC_OUTPUT_SECTION_H

#include <Interface/OutputTypes.h>
#include <Interface/Parset.h>
#include <Interface/SmartPtr.h>
#include <IONProc/OutputThread.h>
#include <Stream/Stream.h>
#include <Thread/Semaphore.h>
#include <Thread/Thread.h>

#include <vector>
#include <string>


namespace LOFAR {
namespace RTCP {

class OutputSection
{
  public:
					   ~OutputSection();

    void				   addIterations(unsigned count);
    void				   noMoreIterations();

  protected:
					   OutputSection(const Parset &, Stream * (*createStream)(unsigned, unsigned), OutputType, const std::vector<unsigned> &cores, int psetIndex, bool integratable);

  private:
    void				   mainLoop();

    void				   droppingData(unsigned subband);
    void				   notDroppingData(unsigned subband);

    std::string               		   itsLogPrefix;

    const unsigned			   itsNrComputeCores;
    const unsigned			   itsNrCoresPerIteration, itsFirstStreamNr, itsNrStreams;
    unsigned				   itsCurrentComputeCore;

    const unsigned			   itsNrIntegrationSteps;
    unsigned				   itsCurrentIntegrationStep;
    unsigned				   itsSequenceNumber;

    const bool                		   itsIsRealTime;
    std::vector<unsigned>		   itsDroppedCount; // [subband]
    std::vector<SmartPtr<OutputThread> >   itsOutputThreads; // [subband]

    std::vector<SmartPtr<Stream> >	   itsStreamsFromCNs;

    std::vector<SmartPtr<StreamableData> > itsSums;
    SmartPtr<StreamableData>		   itsTmpSum;

    Semaphore				   itsNrIterationsToDo;

    SmartPtr<Thread>			   itsThread;
};


class PhaseTwoOutputSection : public OutputSection
{
  protected:
    PhaseTwoOutputSection(const Parset &, Stream * (*createStream)(unsigned, unsigned), OutputType, bool integratable);
};


class PhaseThreeOutputSection : public OutputSection
{
  protected:
    PhaseThreeOutputSection(const Parset &, Stream * (*createStream)(unsigned, unsigned), OutputType);
};


class FilteredDataOutputSection : public PhaseTwoOutputSection
{
  public:
    FilteredDataOutputSection(const Parset &, Stream * (*createStream)(unsigned, unsigned));
};


class CorrelatedDataOutputSection : public PhaseTwoOutputSection
{
  public:
    CorrelatedDataOutputSection(const Parset &, Stream * (*createStream)(unsigned, unsigned));
};


class IncoherentStokesOutputSection : public PhaseTwoOutputSection
{
  public:
    IncoherentStokesOutputSection(const Parset &, Stream * (*createStream)(unsigned, unsigned));
};


class BeamFormedDataOutputSection : public PhaseThreeOutputSection
{
  public:
    BeamFormedDataOutputSection(const Parset &, Stream * (*createStream)(unsigned, unsigned));
};


class CoherentStokesOutputSection : public PhaseThreeOutputSection
{
  public:
    CoherentStokesOutputSection(const Parset &, Stream * (*createStream)(unsigned, unsigned));
};


class TriggerDataOutputSection : public PhaseThreeOutputSection
{
  public:
    TriggerDataOutputSection(const Parset &, Stream * (*createStream)(unsigned, unsigned));
};


}
}

#endif
