//#  AH_InputSection.h: 
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  $Id$
//#
////////////////////////////////////////////////////////////////////

#ifndef AH_INPUTSECTION_H
#define AH_INPUTSECTION_H

#include <CEPFrame/ApplicationHolder.h>

namespace LOFAR {

// This is the ApplicationHolder for the input section of the TFLopCorrelator demo
// This applicationholder uses the CEPFrame library and is supposed to
// connect to the BGLProcessing application Holder (using only tinyCEP). 
// The interface between these is defined in  the SB_Stub class.
// The InputSection receives data from RSP boards using the WH_RSP class.
// This class has an internal buffer and sends output to the WH_Transpose class.
// 

class AH_InputSection: public LOFAR::ApplicationHolder
{
 public:
  AH_InputSection();
  virtual ~AH_InputSection();
  virtual void undefine();
  virtual void define  ();
  virtual void prerun  ();
  virtual void run     (int nsteps);
  virtual void dump    () const;
  virtual void quit    ();
 private:

  vector<WorkHolder*> itsWHs;
  vector<Step*> itsSteps;
  int itsNSBF;                  //number of Subbandfilters (from param file)


};
}
#endif
