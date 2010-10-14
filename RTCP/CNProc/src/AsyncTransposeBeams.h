#ifndef LOFAR_CNPROC_ASYNC_TRANSPOSE_BEAMS_H
#define LOFAR_CNPROC_ASYNC_TRANSPOSE_BEAMS_H

#include <AsyncCommunication.h>
#include <Interface/BeamFormedData.h>
#include <LocationInfo.h>

#if defined HAVE_MPI
#define MPICH_IGNORE_CXX_SEEK
#include <mpi.h>
#endif

#include <vector>


namespace LOFAR {
namespace RTCP {

#if defined HAVE_MPI

// Nodes in input psets read outputPsets.size subbands from their I/O node (one by one).
// Cores communicate with the same logical core number in another pset 
// (due to an extra mapping, this is not the physical core number).

// # sends = size outputPsets (= nrSubbands) on the input nodes.
// # recvs = size inputPsets (= nrStations) on the output nodes.
// Only the output nodes are actually calculating (filtering and correlating).

class AsyncTransposeBeams
{
  public:

  AsyncTransposeBeams(const bool isTransposeInput, const bool isTransposeOutput, unsigned nrSubbands, unsigned nrSubbeams,
	   	      const LocationInfo &, 
		      const std::vector<unsigned> &inputPsets, const std::vector<unsigned> &inputCores, const std::vector<unsigned> &outputPsets, const std::vector<unsigned> &outputCores);
  
  // Post all async receives for the transpose.
  template <typename T, unsigned DIM> void postReceive( SampleData<T,DIM> *transposedData, unsigned subband, unsigned beam, unsigned psetIndex, unsigned coreIndex);
  
  // Wait for a data message. Returns the station number where the message originates.
  unsigned waitForAnyReceive();
  
  // Asynchronously send a subband.
  template <typename T, unsigned DIM> void asyncSend(unsigned outputPsetIndex, unsigned coreIndex, unsigned subband, unsigned beam, unsigned subbeam, const SampleData<T,DIM> *inputData);
  
  // Make sure all async sends have finished.
  void waitForAllSends();
  
 private:
  const bool itsIsTransposeInput, itsIsTransposeOutput;

  AsyncCommunication itsAsyncComm;
  const std::vector<unsigned> itsInputPsets, itsInputCores;
  const std::vector<unsigned> itsOutputPsets, itsOutputCores;
  const LocationInfo &itsLocationInfo;

  // The number of communicates (writes/reads) needed to transport one sub band.
  static const unsigned itsNrCommunications = 1;

  // The maps that contain the handles to the asynchronous reads.
  // The maps are indexed by the inputPset index.
  // The value is -1 if the read finished.
  Matrix<int> itsCommHandles; // [itsNrCommunications][itsNrInputPsets]

  const unsigned itsNrSubbeams;
};

#endif // defined HAVE_MPI

} // namespace RTCP
} // namespace LOFAR

#endif
