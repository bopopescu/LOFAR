//  OnLineProto.cc:
//
//  Copyright (C) 2000, 2002
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dingeloo, The Netherlands, seg@astron.nl
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
//
//////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <Common/lofar_string.h>
#include <Common/KeyValueMap.h>
#include <Common/Debug.h>
#include <Common/lofar_iostream.h>
#include <ACC/ParameterSet.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

//CEPFrame basics
#include <CEPFrame/Step.h>
#include <CEPFrame/Composite.h>
#include <CEPFrame/WH_Empty.h>
#include <CEPFrame/Profiler.h>
#include <OnLineProto/OnLineProto.h>

//WorkHolders:
#include <OnLineProto/WH_SimStation.h>
#include <OnLineProto/WH_PreProcess.h>
#include <OnLineProto/WH_Transpose.h>
#include <OnLineProto/WH_Correlate.h>
#include <OnLineProto/WH_Dump.h>
#include <OnLineProto/WH_FringeControl.h>
#include <OnLineProto/WH_Merge.h>

//DataHolders:
#include <OnLineProto/DH_Beamlet.h>
#include <OnLineProto/DH_CorrCube.h>
#include <OnLineProto/DH_Vis.h>
#include <OnLineProto/DH_CorrectionMatrix.h>

//TransportHolders
#include <Transport/TH_ShMem.h>
#include <Transport/TH_Mem.h>


#include TRANSPORTERINCLUDE
#define nonblocking false

using namespace LOFAR;

OnLineProto::OnLineProto()
{
}

OnLineProto::~OnLineProto()
{
  undefine();
}

/**
   Define function for the OnLineProto application. 
   It defines the static structure of the pipeline
 */
void OnLineProto::define(const KeyValueMap& params)
{

  // the default transportholder
  TH_Mem THproto;

#ifdef HAVE_MPI
  // TH_ShMem only works in combination with MPI
  TH_ShMem::init(0, NULL);
#endif
  
  // Free any memory previously allocated
  undefine();

  // Create the top-level Composite
  Composite app(new WH_Empty(), 
	    "OnLineProto",
	    true, 
	    true,  // controllable	      
	    true); // monitor
  setComposite(app);

  // Set node and application number of Composite
  app.runOnNode(0,0);
  app.setCurAppl(0);

  char str[8];

  // Read in the parameter sets
  const ParameterSet myPS(params.getString("psfile", "params.ps"));

  ////////////////////////////////////////////////////////////////
  //
  // create the station steps
  //
  ////////////////////////////////////////////////////////////////
  WH_SimStation* myWHStations[myPS.getInt("general.nstations")];
  Step*          myStationSteps[myPS.getInt("general.nstations")];

  for (int s=0; s < myPS.getInt("general.nstations"); s++) {
    // ToDo: pass stationID, freq offset etc. to DH
    if (s==0){
      myWHStations[s] = new WH_SimStation("mysimstation",  // name,
					  myPS.getInt("station.nbeamlets"),  // nout
					  string("/home/gerdes/temp/signal_1.txt"),
					  myPS,
					  s);
    } else {
      myWHStations[s] = new WH_SimStation("mysimstation",  // name,
					  myPS.getInt("station.nbeamlets"),  // nout
					  string("/home/gerdes/temp/signal_2.txt"),
					  myPS,
					  s);
    }

    myStationSteps[s] = new Step(myWHStations[s],"mystationstep");
    myStationSteps[s]->runOnNode(0,0);
    app.addStep(myStationSteps[s]);
  }

  ////////////////////////////////////////////////////////////////
  //
  // create the fringe control step
  //
  ////////////////////////////////////////////////////////////////
  WH_FringeControl myWHFringeControl("myfringecontrol", myPS.getInt("general.nstations"), myPS);
  Step myFringeControl(myWHFringeControl, "myfringecontrol_step");
  myFringeControl.runOnNode(0,0);
  app.addStep(myFringeControl);
 
  ////////////////////////////////////////////////////////////////
  //
  // create the merge step
  //
  ////////////////////////////////////////////////////////////////
  WH_Merge myWHMerge("mymerge", 1, myPS.getInt("general.nstations"),
		     myPS.getInt("general.nstations"), 
		     myPS.getInt("station.nchannels"));
  Step myMerge(myWHMerge, "mymergestep");
  myMerge.runOnNode(0,0);
  app.addStep(myMerge);  

  // connect the fringe control step to the merge step
  myMerge.connectInput(&myFringeControl,THproto,nonblocking);

  ////////////////////////////////////////////////////////////////
  //
  // create the preproces steps
  //
  ////////////////////////////////////////////////////////////////
  WH_PreProcess* myWHPreProcess[myPS.getInt("general.nstations")];
  Step*          myPreProcessSteps[myPS.getInt("general.nstations")];

  for (int s=0; s < myPS.getInt("general.nstations"); s++) {
    // ToDo: pass stationID, freq offset etc. to DH
    myWHPreProcess[s] = new WH_PreProcess("mypreprocess",  // name,
					  myPS.getInt("station.nbeamlets"),  // channels
					  myPS,
					  s);

    myPreProcessSteps[s] = new Step(myWHPreProcess[s],"mypreprocess_step");
    myPreProcessSteps[s]->runOnNode(0,0);
    app.addStep(myPreProcessSteps[s]);

    // connect the preprocess step to the station step
    for (int b = 0; b < myPS.getInt("station.nbeamlets"); b++) {
      myPreProcessSteps[s]->connect(myStationSteps[s], b, b, 1, THproto, nonblocking);
    }
     
    // connect the preprocess steps to the fringecontrol step
    myPreProcessSteps[s]->connect(&myMerge, myPS.getInt("station.nbeamlets"), s, 1, THproto, nonblocking);
  }
   
  ////////////////////////////////////////////////////////////////
  //
  // create the Transpose steps
  //
  ////////////////////////////////////////////////////////////////
  WH_Transpose*  myWHTranspose[myPS.getInt("station.nbeamlets")];
  Step*          myTransposeSteps[myPS.getInt("station.nbeamlets")];

   // create a Transpose step for each beamlet (raw freq channel)
  for (int b=0; b < myPS.getInt("station.nbeamlets"); b++) {
    // ToDo: pass stationID, freq offset etc. to DH
    myWHTranspose[b] = new WH_Transpose("mytranspose",  // name,
					myPS.getInt("general.nstations"),  // nin
					myPS.getInt("corr.nvis"), //nout
					myPS
					);


    myTransposeSteps[b] = new Step(myWHTranspose[b],"mytranspose_step");
    myTransposeSteps[b]->runOnNode(0,0);


    myTransposeSteps[b]->setOutRate(myPS.getInt("corr.tsize"));
    app.addStep(myTransposeSteps[b]);
  }


  ////////////////////////////////////////////////////////////////
  //
  // connect the Transpose steps to the preprocessors;
  // connection scheme implements transpose function
  //
  ////////////////////////////////////////////////////////////////
  for (int b = 0; b < myPS.getInt("station.nbeamlets"); b++) {
    for (int s = 0; s < myPS.getInt("general.nstations"); s++) {
      TRACER2("Pipeline!; try to connect " << b << "   " << s);    
      myTransposeSteps[b]->connect(myPreProcessSteps[s],s,b,1, THproto, nonblocking);
    }
  }

  ////////////////////////////////////////////////////////////////
  //
  // create the Correlator steps
  //
  ////////////////////////////////////////////////////////////////
  WH_Correlate*  myWHCorrelators[myPS.getInt("corr.ncorr")];
  Step*          myCorrelatorSteps[myPS.getInt("corr.ncorr")];

  for (int c=0; c <myPS.getInt("station.nbeamlets")* myPS.getInt("corr.ncorr"); c++) {
    // ToDo: pass stationID, freq offset etc. to DH
    myWHCorrelators[c] = new WH_Correlate("mycorrelate",     // name,
					  1,             // channels
					  myPS
					  );
    

    myCorrelatorSteps[c] = new Step(myWHCorrelators[c],"mycorrelate_step");
    myCorrelatorSteps[c]->runOnNode(0,0);
    // todo: only output rate?
    myCorrelatorSteps[c]->getWorker()->getDataManager().setOutputRate(myPS.getInt("corr.tsize"));
    app.addStep(myCorrelatorSteps[c]);
  }

  ////////////////////////////////////////////////////////////////
  //
  // connect the correlator steps to the transpose steps
  //
  ////////////////////////////////////////////////////////////////
  int correlator=0;
  for (int b=0; b<myPS.getInt("station.nbeamlets"); b++) { //NBeamlets
    for (int f=0; f< /*BFBW*/ myPS.getInt("corr.ncorr"); f++) {
      myCorrelatorSteps[correlator]->connect(myTransposeSteps[b],0,f,1, THproto, nonblocking);
      correlator++;
    }
  }
  DbgAssertStr(correlator == myPS.getInt("corr.ncorr"),"error in correlator connection logic");



  ////////////////////////////////////////////////////////////////
  //
  // create the Dump steps
  //
  ////////////////////////////////////////////////////////////////
  WH_Dump*  myWHDumps[myPS.getInt("corr.ncorr")];
  Step*     myDumpSteps[myPS.getInt("corr.ncorr")];


  for (int s=0; s < myPS.getInt("corr.ncorr"); s++) {
    // ToDo: pass stationID, freq offset etc. to DH

    myWHDumps[s] = new WH_Dump("mydump",1,myPS,s);

    myDumpSteps[s] = new Step(myWHDumps[s],"mydump_step");
    myDumpSteps[s]->runOnNode(0,0);

    myDumpSteps[s]->getWorker()->getDataManager().setInputRate(myPS.getInt("corr.tsize"));
    myDumpSteps[s]->getWorker()->getDataManager().setProcessRate(myPS.getInt("corr.tsize"));
    myDumpSteps[s]->getWorker()->getDataManager().setOutputRate(myPS.getInt("corr.tsize"));
    app.addStep(myDumpSteps[s]);
    // connect the preprocess step to the station step
    myDumpSteps[s]->connectInput(myCorrelatorSteps[s], THproto, nonblocking);
  }
}
  

void OnLineProto::run(int nSteps) {
  TRACER1("Call run()");
  Profiler::init();
  Step::clearEventCount();

  TRACER4("Start Processing app");    
  for (int i=0; i<nSteps; i++) {
    if (i==2) Profiler::activate();
    TRACER2("Call app.process() ");
    getComposite().process();
    if (i==5) Profiler::deActivate();
  }

  TRACER4("END OF SIMUL on node " << TRANSPORTER::getCurrentRank () );
 
#if 0
  //     close environment
  TRANSPORTER::finalize();
#endif

}

void OnLineProto::dump() const {
  getComposite().dump();
}

void OnLineProto::quit() {  
}

void OnLineProto::undefine() {
  // Clean up
}
