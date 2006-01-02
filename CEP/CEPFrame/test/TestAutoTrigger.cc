//  TestAutoTrigger.cc: Program for testing simulation parser
//
//  Copyright (C) 2000-2002
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
/////////////////////////////////////////////////////////////////////////

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <TestAutoTrigger.h>
#include <tinyCEP/SimulatorParseClass.h>
#include <CEPFrame/Step.h>
#include <Transport/TH_Mem.h>
//#include <Transport/TH_MPI.h>
#include <CEPFrame/Composite.h>
#include <WH_TestAutoTrigger.h>
#include <tinyCEP/Profiler.h>
#include <Blob/KeyValueMap.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_iostream.h>

using namespace LOFAR;

TestAutoTrigger::~TestAutoTrigger()
{}

void TestAutoTrigger::define (const KeyValueMap& params)
{
  params.show (cout);
//  int rank = TRANSPORTER::getCurrentRank ();
//  unsigned int size = TRANSPORTER::getNumberOfNodes();
//  int appl = Step::getCurAppl ();
//   cout << "CEPFrame Processor " << rank << " of " << size
//        << " operational  (appl=" << appl << ')' << endl;

  // define the top-level simul object
  Composite comp(0, 0, "TestAutoTrigger");
  setComposite (comp);

  // tell the Simul where to run
  comp.runOnNode(0);
  
  // Now start filling the simulation. 
  // first create the Steps
  WH_TestAutoTrigger wh1("Step1");
  Step step1(wh1);
  WH_TestAutoTrigger wh2("Step2");
  Step step2(wh2);

  comp.addBlock(step1);
  comp.addBlock(step2);
// #ifdef HAVE_MPI
//   step2.connectInput(&step1, new TH_MPI(0,0), false);
// #else
  step2.connectInput(&step1, new TH_Mem(), false);
// #endif

  //////////////////////////////////////////////////////////////////////
  //
  // Finished configuration definition
  // 
  //////////////////////////////////////////////////////////////////////
}

void TestAutoTrigger::run (int nsteps)
{
  if (nsteps < 0) {
    nsteps = 25;
  }
  LOG_TRACE_FLOW("Ready with definition of configuration");
  Profiler::init();

  cout << endl <<  "Start Process" << endl;    
  for (int i=0; i<nsteps; i++) {
    getComposite().process();
    dump();
  }
}

void TestAutoTrigger::dump() const
{
  getComposite().dump();
  cout << endl;
}

void TestAutoTrigger::quit()
{

  cout << endl << "END OF SIMUL" << endl;
}



int main (int argc, const char* argv[])
{
  // Set trace level.
  INIT_LOGGER("TestAutoTrigger.log_prop");
  try {
    // First test some Parse functions.
    // Note that C++ also requires a \ to escape special characters.
    ASSERTSTR (SimulatorParse::removeQuotes ("'abcd\"e'\"fgh'i\"'j'")
	       == string("abcd\"efgh'ij"),
	       "SimulatorParse::removeQuotes fails");
    ASSERTSTR (SimulatorParse::removeEscapes ("\\ab\\\\cd\\efghij\\")
	       == string("ab\\cdefghij"),
	       "SimulatorParse::removeEscapes fails");

    TestAutoTrigger simulator;
    simulator.setarg (argc, argv);

    cout << "Welcome to CEPFrame" <<endl;
    cout << "Running in batch mode " << endl;
    cout << endl;
    cout << "Call define" << endl;
    simulator.baseDefine();
    cout << endl;
    cout << "Call run" << endl;
    simulator.baseRun();
    cout << endl;
    cout << "Call dump " << endl;
    simulator.baseDump();
    cout << "Call quit " << endl;
    simulator.baseQuit();
    cout << endl;
    cout << "Good Bye!" << endl;
  } catch (...) {
    cout << "Unexpected exception";
  }
}
