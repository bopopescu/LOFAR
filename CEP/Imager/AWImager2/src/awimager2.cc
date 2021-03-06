//# awimager.cc: Program to create and/or clean a LOFAR image
//# Copyright (C) 2011
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$

//# Includes
#include <lofar_config.h>
#include <AWImager2/Package__Version.h>
#include <AWImager2/ScopedTimer.h>
#include <AWImager2/Imager.h>
#include <AWImager2/Operation.h>
#include <Common/ParameterSet.h>
#include <Common/SystemUtil.h>
#include <Common/Exception.h>
#include <AWImager2/Package__Version.h>
#include <Common/Version.h>
#include <Common/LofarLogger.h>

#include <casacore/images/Images/PagedImage.h>
#include <casacore/images/Images/HDF5Image.h>
#include <casacore/images/Images/ImageFITSConverter.h>
#include <casacore/casa/Arrays/ArrayUtil.h>
#include <casacore/casa/Arrays/ArrayMath.h>
#include <casacore/casa/Arrays/ArrayIter.h>
#include <casacore/casa/Utilities/Regex.h>
#include <casacore/casa/Utilities/Assert.h>
#include <casacore/casa/OS/Directory.h>
#include <casacore/casa/OS/File.h>
#include <casacore/casa/Exceptions/Error.h>
#include <casacore/casa/OS/Timer.h>
#include <casacore/casa/iostream.h>
#include <casacore/casa/sstream.h>

#include <Common/OpenMP.h>
#include <boost/algorithm/string/join.hpp>

using namespace casa;
using LOFAR::operator<<;

// Use a terminate handler that can produce a backtrace.
LOFAR::Exception::TerminateHandler t(LOFAR::Exception::terminate);

IPosition handlePos (const IPosition& pos, const IPosition& def);
void printHelp(Int argc, char** argv, vector<string> operations);
void applyFactors (PagedImage<Float>& image, const Array<Float>& factors);
void correctImages (
  const String& restoName, 
  const String& modelName,
  const String& residName, 
  const String& imgName,
  Imager&,
  Bool CorrectElement, 
  Bool CorrectWPlanes, 
  Bool doRestored);

int main (Int argc, char** argv)
{

  // Fill a string with version info
  LOFAR::Version version_info = LOFAR::AWImager_Version().getInfo();
  string version = version_info.version() + " revision=" + version_info.packageRevision();
  if (version_info.nrChangedFiles() != "0") 
  {
    if (version_info.nrChangedFiles() == "1") 
    {
      version += " (1 locally modified file)";
    }
    else
    {
     version += " (" + version_info.nrChangedFiles() + " locally modified files)";
    }
  }

  // Handle the simple cases first
  // where only version info or a help text needs to be printed
  
  // Print version info
  if (argc>1 && (string(argv[1])=="--version" || string(argv[1])=="version")) 
  {
    cerr<<"awimager version: "<<version<<endl;
    exit(0);
  }

  // Print help info
  vector<string> operations = LOFAR::LofarFT::OperationFactory::instance().registeredClassIds();
  if (argc<=1 || string(argv[1])=="--help" || string(argv[1])=="-help" || string(argv[1])=="help") 
  {
    // printHelp either lists all possible operations
    // or provides help on a specific operation
    printHelp(argc,argv,operations);
    exit(0);
  }


  INIT_LOGGER(LOFAR::basename(string(argv[0])));

  // Now process the parameters
  // The argument(s) can either be the name of a parset file
  // or a list of parameter=value pairs
  
  string parsetname=argv[1];
  LOFAR::ParameterSet parset(true);

  if (parsetname.find('=')==string::npos) {
    parset.adoptFile(parsetname); //case insensitive
  }
  parset.adoptArgv(argc,argv);


  // Get the name of the requested operation
  // The operation itself will read the parameters it needs
  String operation_name = parset.getString("operation");
  LOFAR::LofarFT::Operation *operation = LOFAR::LofarFT::OperationFactory::instance().create(operation_name,parset);
  if (!operation)
  {
    cout << "Unknown operation: " << operation_name << endl;
    cerr << "Valid operations are: \""<<boost::algorithm::join(operations, "\", \"")<<"\""<<endl;
    return 1;
  }

  uint maxthreads = parset.getInt("numthreads",0);
  if (maxthreads) LOFAR::OpenMP::setNumThreads(maxthreads);

  try 
  {
    operation->init();

    // After the initialization of the operation all parameters needed should have been read
    // It is not an error to specify a parameter that will not be used
    // but the user will be warned.
    std::vector<std::string> unused = parset.unusedKeys();
    if (! unused.empty()) {
      LOG_WARN_STR("The following parset keywords were not used");
      LOG_WARN_STR("maybe they are misspelled");
      LOG_WARN_STR(unused);
    }
  
    // Now run the operation
    // This is where the actual work is done
    {
      LOFAR::LofarFT::ScopedTimer t("operation->run()");
      operation->run();
    }
  }
  catch (AipsError& x) 
  {
    std::cerr << "CASA Exception detected: " << x.getMesg() << std::endl;
    return 1;
  }
  // APSExceptions are thrown when an error occurs reading parameters
  catch (LOFAR::APSException& err) 
  {
    // just send err.what() to the error stream
    // this is just the error message, not a full backtrace
    std::cerr << "ParameterSet Exception detected: " << err.what() << std::endl;
    return 1;
  }
  // All other LOFAR Exceptions
  catch (LOFAR::Exception& err) 
  {
    // here send the complete err object to the error stream
    // this can include a backtrace
    std::cerr << "LOFAR Exception detected: " << err << std::endl;
    return 1;
  }
#ifdef __clang__
  catch (std::exception& err) 
  {
    std::cerr << "std exception detected: " << err.what() << std::endl;
    return 1;
  }
#endif

  // Show the status of all scoped timers
  LOFAR::LofarFT::ScopedTimer::show();
  
  LOG_INFO_STR("awimager2 normally ended");
  return 0;
}


void printHelp(Int argc, char** argv, vector<string> operations) {
  cerr<<"usage: awimager file.parset [parsetkeys]"<<endl<<endl;
  if (argc<=2)
  {
    cerr<<"  For information on what should be in the parset, type        "<<endl
        <<"    awimager help <operation>"<<endl
        <<"  Valid operations: \""<<boost::algorithm::join(operations, "\", \"")<<"\""<<endl;
  } else
  {
    String operation_name = argv[2];
    LOFAR::ParameterSet emptyparset;
    LOFAR::LofarFT::Operation *operation =
        LOFAR::LofarFT::OperationFactory::instance().create(operation_name,emptyparset);
    if (!operation)
    {
      cerr << "  Unknown operation \"" << operation_name << "\""<<endl;
      cerr << "  Valid operations are: \""<<boost::algorithm::join(operations, "\", \"")<<"\""<<endl;
      return;
    }

    operation->showHelp(cerr,operation_name);
  }
}

IPosition handlePos (const IPosition& pos, const IPosition& def)
{
  if (pos.nelements() == 0) {
    return def;
  }
  if (pos.nelements() != 2) {
    throw AipsError("Give 0 or 2 values in maskblc and masktrc");
  }
  IPosition npos(def);
  int n = npos.nelements();
  npos[n-1] = pos[1];
  npos[n-2]= pos[0];
  return npos;
}

IPosition readIPosition (const String& in)
{
  if (in.empty()) {
    return IPosition();
  }
  Vector<String> strs = stringToVector (in);
  IPosition ipos(strs.nelements());
  for (uInt i=0; i<strs.nelements(); ++i) {
    istringstream iss(strs[i]);
    iss >> ipos[i];
  }
  return ipos;
}




void applyFactors (PagedImage<Float>& image, const Array<Float>& factors)
{
  Array<Float> data;
  image.get (data);
  ///  cout << "apply factor to " << data.data()[0] << ' ' << factors.data()[0]<<endl;
  // Loop over channels
  for (ArrayIterator<Float> iter1(data, 3); !iter1.pastEnd(); iter1.next()) {
    // Loop over Stokes.
    ArrayIterator<Float> iter2(iter1.array(), 2);
    while (! iter2.pastEnd()) {
      iter2.array() *= factors;
      iter2.next();
    }
  }
  image.put (data);
  ///  cout << "applied factor to " << data.data()[0] << ' ' << factors.data()[0]<<endl;
}

void correctImages (
  const String& restoName, 
  const String& modelName,
  const String& residName, 
  const String& imgName,
  Imager&,
  Bool CorrectElement, 
  Bool CorrectWPlanes, 
  Bool doRestored)
{
//   // Copy the images to .corr ones.
//   {
//     Directory modelIn(modelName);
//     modelIn.copy (modelName+".corr");
//     Directory residualIn(residName);
//     residualIn.copy (residName+".corr");
//   }
//   // Open the images.
//   PagedImage<Float> modelImage(modelName+".corr");
//   PagedImage<Float> residualImage(residName+".corr");
//   // AlwaysAssert (residualImage.shape() == modelImage.shape()  &&
//   //              restoredImage.shape() == modelImage.shape(), SynthesisError);
// 
//   // Get average primary beam and spheroidal.
// //   Matrix<Float> avgPB = LOFAR::LofarFT::ConvolutionFunction::getAveragePB(imgName+"0");
// //   Matrix<Float> spheroidCut = LOFAR::LofarFT::ConvolutionFunction::getSpheroidCut(imgName+"0");
//   Matrix<Float> avgPB = LOFAR::LofarFT::ConvolutionFunction::getAveragePB(imgName);
//   Matrix<Float> spheroidCut = LOFAR::LofarFT::ConvolutionFunction::getSpheroidCut(imgName);
// 
//   // String nameii("Spheroid_cut_im_element.img");
//   // ostringstream nameiii(nameii);
//   // PagedImage<Float> tmpi(nameiii.str().c_str());
//   // Slicer slicei(IPosition(4,0,0,0,0), tmpi.shape(), IPosition(4,1,1,1,1));
//   // Array<Float> spheroidCutElement;
//   // tmpi.doGetSlice(spheroidCutElement, slicei);
// 
//   // Use the inner part of the beam and spheroidal.
//   Int nximg = residualImage.shape()[0];
//   Int nxpb  = avgPB.shape()[0];
//   Int nxsph = spheroidCut.shape()[0];
//   //AlwaysAssert (restoredImage.shape()[1] == nximg  &&
//   //              avgPB.shape()[1] == nxpb  &&
//   //              spheroidCut.shape()[1] == nxsph  &&
//   //              nxsph >= nximg  &&  nxpb >= nximg, SynthesisError);
//   // Get inner parts of beam and spheroid.
//   Int offpb  = (nxpb  - nximg) / 2;
//   Int offsph = (nxsph - nximg) / 2;
//   Array<Float> pbinner  = avgPB(Slicer(IPosition(2, offpb, offpb),
//                                        IPosition(2, nximg, nximg)));
//   Array<Float> sphinner = spheroidCut(Slicer(IPosition(2, offsph, offsph),
//                                              IPosition(2, nximg, nximg)));
//   Array<Float> ones(IPosition(sphinner.shape()),1.);
//   Array<Float> factors;
// 
//   factors = ones / sqrt(pbinner);
//   cout<<"Final normalisation"<<endl;
//   applyFactors (modelImage, factors);
//   applyFactors (residualImage, factors);
//   if(doRestored){
//     cout<<"... restored image too ..."<<endl;
//     Directory restoredIn(restoName);
//     restoredIn.copy (restoName+".corr");
//     PagedImage<Float> restoredImage(restoName+".corr");
//     applyFactors (restoredImage, factors);
//   }
}
