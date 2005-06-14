//#  TestFilter.cc: main test filter program
//#
//#  Copyright (C) 2002-2005
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

#include <TestFilter.h>

#include <Transport/TH_MPI.h>
#include <tinyCEP/SimulatorParseClass.h>

using namespace LOFAR;

int main (int argc, const char** argv){

  try {
#ifdef HAVE_MPI
    TH_MPI::init(argc, argv);
#endif
    
    AH_TestFilter app;
    app.setarg(argc,argv);
    app.baseDefine();
    cout << "define done" << endl;
    app.basePrerun();
    cout << "Pre run done" << endl;
    app.baseRun(1);
    cout << "Run done" << endl;
    app.baseDump();
    app.baseQuit();

#ifdef HAVE_MPI
    TH_MPI::finalize();
#endif
  } catch (LOFAR::Exception ex) {
    cout << "Caught exception: "<< ex.what() << endl;
  } catch (...) {
    cout << "Caught exception "<< endl;
  }
  return 0;
}
