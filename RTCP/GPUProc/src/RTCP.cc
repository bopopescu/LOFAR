#include "lofar_config.h"

#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>

#include <omp.h>

#include <cmath>
#include <complex>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <boost/multi_array.hpp>

#include "Align.h"
#include "BandPass.h"
#include "Common/lofar_datetime.h"
#include "Common/LofarLogger.h"
#include "Common/SystemUtil.h"
#include "FilterBank.h"
#include "InputSection.h"
#include "Interface/Parset.h"
#include "OpenCL_FFT/clFFT.h"
#include "OpenCL_Support.h"
#include "OpenMP_Support.h"
#include "UHEP/InvertedStationPPFWeights.h"
//#include "clAmdFft/include/clAmdFft.h"

namespace LOFAR {
namespace RTCP {

bool	 profiling = true;
unsigned nrGPUs;

//#define NR_BITS_PER_SAMPLE	 8
#define NR_POLARIZATIONS	 2
#define NR_TAPS			16
#define NR_STATION_FILTER_TAPS	16

// the SAP to process (we support only one SAP for now)
#define SAP 0

#define USE_2X2
#undef USE_CUSTOM_FFT
#undef USE_TEST_DATA
#undef USE_B7015


double getTime()
{
  static double firstTime = 0.0;

#if defined __linux__
  struct timeval tv;

  if (gettimeofday(&tv, 0) < 0) {
    perror("gettimeofday");
    exit(1);
  }

  double now = tv.tv_sec + tv.tv_usec / 1e6;
#elif defined _WIN32 || defined __WIN32__ || defined _WIN64
  static LARGE_INTEGER freq;

  if (firstTime == 0 && !QueryPerformanceFrequency(&freq))
    std::cerr << "No high-resolution timer available" << std::endl;

  LARGE_INTEGER time;
  QueryPerformanceCounter(&time);

  double now = (double) time.QuadPart / (double) freq.QuadPart;
#endif

  if (firstTime == 0.0)
    firstTime = now;

  return now - firstTime;
}


class PerformanceCounter
{
  public:
    PerformanceCounter(const std::string &name);
    ~PerformanceCounter();

    void doOperation(cl::Event &, size_t nrOperations, size_t nrBytesRead, size_t nrBytesWritten);

  private:
    static void eventCompleteCallBack(cl_event, cl_int /*status*/, void *counter);

    size_t	      totalNrOperations, totalNrBytesRead, totalNrBytesWritten;
    double	      totalTime;
    unsigned	      totalEvents;
    const std::string name;
};


PerformanceCounter::PerformanceCounter(const std::string &name)
:
  totalNrOperations(0),
  totalNrBytesRead(0),
  totalNrBytesWritten(0),
  totalTime(0),
  totalEvents(0),
  name(name)
{
}


PerformanceCounter::~PerformanceCounter()
{
  if (totalTime > 0)
#pragma omp critical (cout)
    std::cout << std::setw(12) << name
	      << std::setprecision(3)
	      << ": avg. time = " << 1000 * totalTime / totalEvents << " ms, "
		 "GFLOP/s = " << totalNrOperations / totalTime / 1e9 << ", "
		 "R/W = " << totalNrBytesRead / totalTime / 1e9 << '+'
	      << totalNrBytesWritten / totalTime / 1e9 << '='
	      << (totalNrBytesRead + totalNrBytesWritten) / totalTime / 1e9 << " GB/s"
	      << std::endl;
}


void PerformanceCounter::eventCompleteCallBack(cl_event ev, cl_int /*status*/, void *counter)
{
  cl::Event event(ev);

  size_t queued, submitted, start, stop;
  event.getProfilingInfo(CL_PROFILING_COMMAND_QUEUED, &queued);
  event.getProfilingInfo(CL_PROFILING_COMMAND_SUBMIT, &submitted);
  event.getProfilingInfo(CL_PROFILING_COMMAND_START, &start);
  event.getProfilingInfo(CL_PROFILING_COMMAND_END, &stop);
  double seconds = (stop - start) / 1e9;

  if (seconds < 0 || seconds > 15)
#pragma omp critical (cout)
    std::cout << "BAH! " << omp_get_thread_num() << ": " << queued << ' ' << submitted - queued << ' ' << start - queued << ' ' << stop - queued << std::endl;

#pragma omp atomic
  static_cast<PerformanceCounter *>(counter)->totalTime += seconds;

  // cl::~Event() decreases ref count
}


void PerformanceCounter::doOperation(cl::Event &event, size_t nrOperations, size_t nrBytesRead, size_t nrBytesWritten)
{
  // reference count between C and C++ conversions is serously broken in C++ wrapper
  cl_event ev = event();
  cl_int error = clRetainEvent(ev);

  if (error != CL_SUCCESS)
    throw cl::Error(error, "clRetainEvent");

  if (profiling) {
    event.setCallback(CL_COMPLETE, &PerformanceCounter::eventCompleteCallBack, this);

#pragma omp atomic
    totalNrOperations   += nrOperations;
#pragma omp atomic
    totalNrBytesRead    += nrBytesRead;
#pragma omp atomic
    totalNrBytesWritten += nrBytesWritten;
#pragma omp atomic
    ++ totalEvents;
  }
}


cl::Program createProgram(const Parset &ps, cl::Context &context, std::vector<cl::Device> &devices, const char *sources)
{
  std::stringstream args;
  args << "-cl-fast-relaxed-math";

  std::vector<cl_context_properties> properties;
  context.getInfo(CL_CONTEXT_PROPERTIES, &properties);

  if (cl::Platform((cl_platform_id) properties[1]).getInfo<CL_PLATFORM_NAME>() == "NVIDIA CUDA") {
    args << " -cl-nv-verbose";
    args << " -cl-nv-opt-level=99";
    //args << " -cl-nv-maxrregcount=63";
    args << " -DNVIDIA_CUDA";
  }

  //if (devices[0].getInfo<CL_DEVICE_NAME>() == "GeForce GTX 680")
    //args << " -DUSE_FLOAT4_IN_CORRELATOR";

  args << " -I" << dirname(__FILE__);
  args << " -DNR_BITS_PER_SAMPLE=" << ps.nrBitsPerSample();
  args << " -DSUBBAND_BANDWIDTH=" << std::setprecision(7) << ps.subbandBandwidth() << 'f';
  args << " -DNR_SUBBANDS=" << ps.nrSubbands();
  args << " -DNR_CHANNELS=" << ps.nrChannelsPerSubband();
  args << " -DNR_STATIONS=" << ps.nrStations();
  args << " -DNR_SAMPLES_PER_CHANNEL=" << ps.nrSamplesPerChannel();
  args << " -DNR_SAMPLES_PER_SUBBAND=" << ps.nrSamplesPerSubband();
  args << " -DNR_BEAMS=" << ps.nrBeams();
  args << " -DNR_TABS=" << ps.nrTABs(SAP);
  args << " -DNR_COHERENT_STOKES=" << ps.nrCoherentStokes();
  args << " -DNR_INCOHERENT_STOKES=" << ps.nrIncoherentStokes();
  args << " -DCOHERENT_STOKES_TIME_INTEGRATION_FACTOR=" << ps.coherentStokesTimeIntegrationFactor();
  args << " -DINCOHERENT_STOKES_TIME_INTEGRATION_FACTOR=" << ps.incoherentStokesTimeIntegrationFactor();
  args << " -DNR_POLARIZATIONS=" << NR_POLARIZATIONS;
  args << " -DNR_TAPS=" << NR_TAPS;
  args << " -DNR_STATION_FILTER_TAPS=" << NR_STATION_FILTER_TAPS;

  if (ps.delayCompensation())
    args << " -DDELAY_COMPENSATION";

  if (ps.correctBandPass())
    args << " -DBANDPASS_CORRECTION";

  args << " -DDEDISPERSION_FFT_SIZE=" << ps.dedispersionFFTsize();
  return createProgram(context, devices, dirname(__FILE__).append("/").append(sources).c_str(), args.str().c_str());
}


class FFT_Plan
{
  public:
    FFT_Plan(cl::Context &context, unsigned fftSize)
    {
      clFFT_Dim3 dim = { fftSize, 1, 1 };
      cl_int error;
      plan = clFFT_CreatePlan(context(), dim, clFFT_1D, clFFT_InterleavedComplexFormat, &error);

      if (error != CL_SUCCESS)
	throw cl::Error(error, "clFFT_CreatePlan");

      //clFFT_DumpPlan(plan, stdout);
    }

    ~FFT_Plan()
    {
      clFFT_DestroyPlan(plan);
    }

    clFFT_Plan plan;
};


class Pipeline
{
  public:
			    Pipeline(const Parset &);

    cl::Program		    createProgram(const char *sources);

    const Parset	    &ps;
    cl::Context		    context;
    std::vector<cl::Device> devices;
    SmartPtr<InputSection<i16complex> > inputSection16;
    SmartPtr<InputSection<i8complex> >	inputSection8;
    SmartPtr<InputSection<i4complex> >	inputSection4;
};


class CorrelatorPipeline : public Pipeline
{
  public:
			    CorrelatorPipeline(const Parset &);

    void		    doWork();

  //private:
    //friend class CorrelatorWorkQueue;

    FilterBank		    filterBank;

    cl::Program		    firFilterProgram, delayAndBandPassProgram, correlatorProgram;
    PerformanceCounter	    firFilterCounter, delayAndBandPassCounter, correlatorCounter, fftCounter;
    PerformanceCounter	    samplesCounter, visibilitiesCounter;

#if defined USE_B7015
    OMP_Lock hostToDeviceLock[4], deviceToHostLock[4];
#endif
};


class BeamFormerPipeline : public Pipeline
{
  public:
			    BeamFormerPipeline(const Parset &);

    void		    doWork();

    cl::Program		    intToFloatProgram, delayAndBandPassProgram, beamFormerProgram, transposeProgram, dedispersionChirpProgram;

    PerformanceCounter	    intToFloatCounter, fftCounter, delayAndBandPassCounter, beamFormerCounter, transposeCounter, dedispersionForwardFFTcounter, dedispersionChirpCounter, dedispersionBackwardFFTcounter;
    PerformanceCounter	    samplesCounter;
};


class UHEP_Pipeline : public Pipeline
{
  public:
			    UHEP_Pipeline(const Parset &);

    void		    doWork();

    cl::Program		    beamFormerProgram, transposeProgram, invFFTprogram, invFIRfilterProgram, triggerProgram;
    PerformanceCounter	    beamFormerCounter, transposeCounter, invFFTcounter, invFIRfilterCounter, triggerCounter;
    PerformanceCounter	    beamFormerWeightsCounter, samplesCounter;
};


Pipeline::Pipeline(const Parset &ps)
:
  ps(ps)//,
  //inputSection16(ps.nrBitsPerSample() == 16 ? new InputSection<i16complex>(ps, 0) : 0),
  //inputSection8(ps.nrBitsPerSample() == 8 ? new InputSection<i8complex>(ps, 0) : 0),
  //inputSection4(ps.nrBitsPerSample() == 4 ? new InputSection<i4complex>(ps, 0) : 0)
{
  createContext(context, devices);
}


cl::Program Pipeline::createProgram(const char *sources)
{
  return LOFAR::RTCP::createProgram(ps, context, devices, sources);
}


CorrelatorPipeline::CorrelatorPipeline(const Parset &ps)
:
  Pipeline(ps),
  filterBank(true, NR_TAPS, ps.nrChannelsPerSubband(), KAISER),
  firFilterCounter("FIR filter"),
  delayAndBandPassCounter("delay/bp"),
  correlatorCounter("correlator"),
  fftCounter("FFT"),
  samplesCounter("samples"),
  visibilitiesCounter("visibilities")
{
  filterBank.negateWeights();

  double startTime = getTime();

#pragma omp parallel sections
  {
#pragma omp section
    firFilterProgram = createProgram("FIR.cl");
#pragma omp section
    delayAndBandPassProgram = createProgram("DelayAndBandPass.cl");
#pragma omp section
    correlatorProgram = createProgram("NewCorrelator.cl");
    //correlatorProgram = createProgram("Correlator.cl");
  }

  std::cout << "compile time = " << getTime() - startTime << std::endl;
}


BeamFormerPipeline::BeamFormerPipeline(const Parset &ps)
:
  Pipeline(ps),
  intToFloatCounter("int-to-float"),
  fftCounter("FFT"),
  delayAndBandPassCounter("delay/bp"),
  beamFormerCounter("beamformer"),
  transposeCounter("transpose"),
  dedispersionForwardFFTcounter("ddisp.fw.FFT"),
  dedispersionChirpCounter("chirp"),
  dedispersionBackwardFFTcounter("ddisp.bw.FFT"),
  samplesCounter("samples")
{
  double startTime = getTime();

#pragma omp parallel sections
  {
#pragma omp section
    intToFloatProgram = createProgram("BeamFormer/IntToFloat.cl");
#pragma omp section
    delayAndBandPassProgram = createProgram("DelayAndBandPass.cl");
#pragma omp section
    beamFormerProgram = createProgram("BeamFormer/BeamFormer.cl");
#pragma omp section
    transposeProgram = createProgram("BeamFormer/Transpose.cl");
#pragma omp section
    dedispersionChirpProgram = createProgram("BeamFormer/Dedispersion.cl");
  }

  std::cout << "compile time = " << getTime() - startTime << std::endl;
}


UHEP_Pipeline::UHEP_Pipeline(const Parset &ps)
:
  Pipeline(ps),
  beamFormerCounter("beamformer"),
  transposeCounter("transpose"),
  invFFTcounter("inv. FFT"),
  invFIRfilterCounter("inv. FIR"),
  triggerCounter("trigger"),
  beamFormerWeightsCounter("BF weights"),
  samplesCounter("samples")
{
  double startTime = getTime();

#pragma omp parallel sections
  {
#pragma omp section
    beamFormerProgram = createProgram("UHEP/BeamFormer.cl");
#pragma omp section
    transposeProgram = createProgram("UHEP/Transpose.cl");
#pragma omp section
    invFFTprogram = createProgram("UHEP/InvFFT.cl");
#pragma omp section
    invFIRfilterProgram = createProgram("UHEP/InvFIR.cl");
#pragma omp section
    triggerProgram = createProgram("UHEP/Trigger.cl");
  }

  std::cout << "compile time = " << getTime() - startTime << std::endl;
}


class WorkQueue
{
  public:
    WorkQueue(Pipeline &);

    const unsigned	gpu;
    cl::Device		&device;
    cl::CommandQueue	queue;

  protected:
    const Parset	&ps;
};


class CorrelatorWorkQueue : public WorkQueue
{
  public:
    CorrelatorWorkQueue(CorrelatorPipeline &);

    void doWork();

#if defined USE_TEST_DATA
    void setTestPattern();
    void printTestOutput();
#endif

  //private:
    CorrelatorPipeline	&pipeline;
    cl::Buffer		devFIRweights;
    cl::Buffer		devBufferA, devBufferB;
    MultiArraySharedBuffer<float, 1> bandPassCorrectionWeights;
    MultiArraySharedBuffer<float, 3> delaysAtBegin, delaysAfterEnd;
    MultiArraySharedBuffer<float, 2> phaseOffsets;
    MultiArraySharedBuffer<char, 4> inputSamples;

    cl::Buffer		devFilteredData;
    cl::Buffer		devCorrectedData;

    MultiArraySharedBuffer<std::complex<float>, 4> visibilities;
};


class BeamFormerWorkQueue : public WorkQueue
{
  public:
    BeamFormerWorkQueue(BeamFormerPipeline &);

    void doWork();

    BeamFormerPipeline	&pipeline;

    MultiArraySharedBuffer<char, 4>	   inputSamples;
    cl::Buffer					   devFilteredData;
    MultiArraySharedBuffer<float, 1>		   bandPassCorrectionWeights;
    MultiArraySharedBuffer<float, 3>		   delaysAtBegin, delaysAfterEnd;
    MultiArraySharedBuffer<float, 2>		   phaseOffsets;
    cl::Buffer					   devCorrectedData;
    MultiArraySharedBuffer<std::complex<float>, 3> beamFormerWeights;
    cl::Buffer					   devComplexVoltages;
    MultiArraySharedBuffer<std::complex<float>, 4> transposedComplexVoltages;
    MultiArraySharedBuffer<float, 1>		   DMs;
};


struct TriggerInfo {
  float	   mean, variance, bestValue;
  unsigned bestApproxIndex;
};

class UHEP_WorkQueue : public WorkQueue
{
  public:
    UHEP_WorkQueue(UHEP_Pipeline &);

    void doWork(const float *delaysAtBegin, const float *delaysAfterEnd, const float *phaseOffsets);

    UHEP_Pipeline	&pipeline;
    cl::Event		inputSamplesEvent, beamFormerWeightsEvent;

    cl::Buffer		devBuffers[2];
    cl::Buffer		devInputSamples;
    MultiArrayHostBuffer<char, 5> hostInputSamples;

    cl::Buffer		devBeamFormerWeights;
    MultiArrayHostBuffer<std::complex<float>, 3> hostBeamFormerWeights;

    cl::Buffer		devComplexVoltages;
    cl::Buffer		devReverseSubbandMapping;
    cl::Buffer		devFFTedData;
    cl::Buffer		devInvFIRfilteredData;
    cl::Buffer		devInvFIRfilterWeights;

    cl::Buffer		devTriggerInfo;
    VectorHostBuffer<TriggerInfo> hostTriggerInfo;
};


class Kernel : public cl::Kernel
{
  public:
    Kernel(const Parset &ps, cl::Program &program, const char *name)
    :
      cl::Kernel(program, name),
      ps(ps)
    {
    }

    void enqueue(cl::CommandQueue &queue, PerformanceCounter &counter)
    {
      queue.enqueueNDRangeKernel(*this, cl::NullRange, globalWorkSize, localWorkSize, 0, &event);
      counter.doOperation(event, nrOperations, nrBytesRead, nrBytesWritten);
    }

  protected:
    cl::Event	 event;
    const Parset &ps;
    cl::NDRange  globalWorkSize, localWorkSize;
    size_t       nrOperations, nrBytesRead, nrBytesWritten;
};


class FIR_FilterKernel : public Kernel
{
  public:
    FIR_FilterKernel(const Parset &ps, cl::CommandQueue &queue, cl::Program &program, cl::Buffer &devFilteredData, cl::Buffer &devInputSamples, cl::Buffer &devFIRweights)
    :
      Kernel(ps, program, "FIR_filter")
    {
      setArg(0, devFilteredData);
      setArg(1, devInputSamples);
      setArg(2, devFIRweights);

      size_t maxNrThreads;
      getWorkGroupInfo(queue.getInfo<CL_QUEUE_DEVICE>(), CL_KERNEL_WORK_GROUP_SIZE, &maxNrThreads);
      unsigned totalNrThreads = ps.nrChannelsPerSubband() * NR_POLARIZATIONS * 2;
      unsigned nrPasses = (totalNrThreads + maxNrThreads - 1) / maxNrThreads;
      globalWorkSize = cl::NDRange(totalNrThreads, ps.nrStations());
      localWorkSize  = cl::NDRange(totalNrThreads / nrPasses, 1);

      size_t nrSamples = (size_t) ps.nrStations() * ps.nrChannelsPerSubband() * NR_POLARIZATIONS;
      nrOperations   = nrSamples * ps.nrSamplesPerChannel() * NR_TAPS * 2 * 2;
      nrBytesRead    = nrSamples * (NR_TAPS - 1 + ps.nrSamplesPerChannel()) * ps.nrBytesPerComplexSample();
      nrBytesWritten = nrSamples * ps.nrSamplesPerChannel() * sizeof(std::complex<float>);
    }
};


class FFT_Kernel
{
  public:
    FFT_Kernel(cl::Context &context, unsigned fftSize, unsigned nrFFTs, bool forward, cl::Buffer &buffer)
    :
      nrFFTs(nrFFTs),
      fftSize(fftSize)
#if defined USE_CUSTOM_FFT
    {
      ASSERT(fftSize == 256);
      ASSERT(forward);
      std::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
      cl::Program program = createProgram(context, devices, "FFT.cl", "");
      kernel = cl::Kernel(program, "fft0");
      kernel.setArg(0, buffer);
    }
#else
    , direction(forward ? clFFT_Forward : clFFT_Inverse),
      plan(context, fftSize),
      buffer(buffer)
    {
    }
#endif

    void enqueue(cl::CommandQueue &queue, PerformanceCounter &counter)
    {
#if defined USE_CUSTOM_FFT
      queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(nrFFTs * 64 / 4, 4), cl::NDRange(64, 4), 0, &event);
#else
      cl_int error = clFFT_ExecuteInterleaved(queue(), plan.plan, nrFFTs, direction, buffer(), buffer(), 0, 0, &event());

      if (error != CL_SUCCESS)
	throw cl::Error(error, "clFFT_ExecuteInterleaved");
#endif

      counter.doOperation(event,
	(size_t) nrFFTs * 5 * fftSize * log2(fftSize),
	(size_t) nrFFTs * fftSize * sizeof(std::complex<float>),
	(size_t) nrFFTs * fftSize * sizeof(std::complex<float>));
    }

  private:
    unsigned	 nrFFTs, fftSize;
#if defined USE_CUSTOM_FFT
    cl::Kernel	 kernel;
#else
    clFFT_Direction direction;
    FFT_Plan     plan;
    cl::Buffer	 &buffer;
#endif 
    cl::Event	 event;
};


class Filter_FFT_Kernel : public FFT_Kernel
{
  public:
    Filter_FFT_Kernel(const Parset &ps, cl::Context &context, cl::Buffer &devFilteredData)
    :
      FFT_Kernel(context, ps.nrChannelsPerSubband(), ps.nrStations() * NR_POLARIZATIONS * ps.nrSamplesPerChannel(), true, devFilteredData)
    {
    }
};


class DelayAndBandPassKernel : public Kernel
{
  public:
    DelayAndBandPassKernel(const Parset &ps, cl::Program &program, cl::Buffer &devCorrectedData, cl::Buffer &devFilteredData, cl::Buffer &devDelaysAtBegin, cl::Buffer &devDelaysAfterEnd, cl::Buffer &devPhaseOffsets, cl::Buffer &devBandPassCorrectionWeights)
    :
      Kernel(ps, program, "applyDelaysAndCorrectBandPass")
    {
      ASSERT(ps.nrChannelsPerSubband() % 16 == 0 || ps.nrChannelsPerSubband() == 1);
      ASSERT(ps.nrSamplesPerChannel() % 16 == 0);

      setArg(0, devCorrectedData);
      setArg(1, devFilteredData);
      setArg(4, devDelaysAtBegin);
      setArg(5, devDelaysAfterEnd);
      setArg(6, devPhaseOffsets);
      setArg(7, devBandPassCorrectionWeights);

      globalWorkSize = cl::NDRange(256, ps.nrChannelsPerSubband() == 1 ? 1 : ps.nrChannelsPerSubband() / 16, ps.nrStations());
      localWorkSize  = cl::NDRange(256, 1, 1);

      size_t nrSamples = ps.nrStations() * ps.nrChannelsPerSubband() * ps.nrSamplesPerChannel() * NR_POLARIZATIONS;
      nrOperations = nrSamples * 12;
      nrBytesRead = nrBytesWritten = nrSamples * sizeof(std::complex<float>);
    }

    void enqueue(cl::CommandQueue &queue, PerformanceCounter &counter, unsigned subband)
    {
      setArg(2, (float) ps.subbandToFrequencyMapping()[subband]);
      setArg(3, 0); // beam
      Kernel::enqueue(queue, counter);
    }
};


#if 0

class CorrelatorKernel : public Kernel
{
  public:
    CorrelatorKernel(const Parset &ps, cl::CommandQueue &queue, cl::Program &program, cl::Buffer &devVisibilities, cl::Buffer &devCorrectedData)
    :
#if defined USE_4X4
      Kernel(ps, program, "correlate_4x4")
#elif defined USE_3X3
      Kernel(ps, program, "correlate_3x3")
#elif defined USE_2X2
      Kernel(ps, program, "correlate_2x2")
#else
      Kernel(ps, program, "correlate")
#endif
    {
      setArg(0, devVisibilities);
      setArg(1, devCorrectedData);

      size_t maxNrThreads, preferredMultiple;
      getWorkGroupInfo(queue.getInfo<CL_QUEUE_DEVICE>(), CL_KERNEL_WORK_GROUP_SIZE, &maxNrThreads);

      std::vector<cl_context_properties> properties;
      queue.getInfo<CL_QUEUE_CONTEXT>().getInfo(CL_CONTEXT_PROPERTIES, &properties);

      if (cl::Platform((cl_platform_id) properties[1]).getInfo<CL_PLATFORM_NAME>() == "AMD Accelerated Parallel Processing")
	preferredMultiple = 256;
      else
	getWorkGroupInfo(queue.getInfo<CL_QUEUE_DEVICE>(), CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, &preferredMultiple);

#if defined USE_4X4
      unsigned quartStations = (ps.nrStations() + 2) / 4;
      unsigned nrBlocks = quartStations * (quartStations + 1) / 2;
#elif defined USE_3X3
      unsigned thirdStations = (ps.nrStations() + 2) / 3;
      unsigned nrBlocks = thirdStations * (thirdStations + 1) / 2;
#elif defined USE_2X2
      unsigned halfStations = (ps.nrStations() + 1) / 2;
      unsigned nrBlocks = halfStations * (halfStations + 1) / 2;
#else
      unsigned nrBlocks = ps.nrBaselines();
#endif
      unsigned nrPasses = (nrBlocks + maxNrThreads - 1) / maxNrThreads;
      unsigned nrThreads = (nrBlocks + nrPasses - 1) / nrPasses;
      nrThreads = (nrThreads + preferredMultiple - 1) / preferredMultiple * preferredMultiple;
      //std::cout << "nrBlocks = " << nrBlocks << ", nrPasses = " << nrPasses << ", preferredMultiple = " << preferredMultiple << ", nrThreads = " << nrThreads << std::endl;

      globalWorkSize = cl::NDRange(nrPasses * nrThreads, ps.nrChannelsPerSubband());
      localWorkSize  = cl::NDRange(nrThreads, 1);

      nrOperations   = (size_t) ps.nrChannelsPerSubband() * ps.nrBaselines() * ps.nrSamplesPerChannel() * 32;
      nrBytesRead    = (size_t) nrPasses * ps.nrStations() * ps.nrChannelsPerSubband() * ps.nrSamplesPerChannel() * NR_POLARIZATIONS * sizeof(std::complex<float>);
      nrBytesWritten = (size_t) ps.nrBaselines() * ps.nrChannelsPerSubband() * NR_POLARIZATIONS * NR_POLARIZATIONS * sizeof(std::complex<float>);
    }
};

#else

class CorrelatorKernel : public Kernel
{
  public:
    CorrelatorKernel(const Parset &ps, cl::CommandQueue &queue, cl::Program &program, cl::Buffer &devVisibilities, cl::Buffer &devCorrectedData)
    :
#if defined USE_2X2
      Kernel(ps, program, "correlateRectangles")
#else
#error not implemented
#endif
    {
      setArg(0, devVisibilities);
      setArg(1, devCorrectedData);

      unsigned nrRectanglesPerSide = ((ps.nrStations() - 1) / (2 * 16));
      unsigned nrRectangles = nrRectanglesPerSide * (nrRectanglesPerSide + 1) / 2;
#pragma omp critical (cout)
      std::cout << "nrRectangles = " << nrRectangles << std::endl;

      globalWorkSize = cl::NDRange(16 * 16, nrRectangles, ps.nrChannelsPerSubband());
      localWorkSize  = cl::NDRange(16 * 16, 1, 1);

      nrOperations   = (size_t) (32 * 32) * nrRectangles * ps.nrChannelsPerSubband() * ps.nrSamplesPerChannel() * 32;
      nrBytesRead    = (size_t) (32 + 32) * nrRectangles * ps.nrChannelsPerSubband() * ps.nrSamplesPerChannel() * NR_POLARIZATIONS * sizeof(std::complex<float>);
      nrBytesWritten = (size_t) (32 * 32) * nrRectangles * ps.nrChannelsPerSubband() * NR_POLARIZATIONS * NR_POLARIZATIONS * sizeof(std::complex<float>);
    }
};

#endif


class IntToFloatKernel : public Kernel
{
  public:
    IntToFloatKernel(const Parset &ps, cl::CommandQueue &queue, cl::Program &program, cl::Buffer &devFilteredData, cl::Buffer &devInputSamples)
    :
      Kernel(ps, program, "intToFloat")
    {
      setArg(0, devFilteredData);
      setArg(1, devInputSamples);

      size_t maxNrThreads;
      getWorkGroupInfo(queue.getInfo<CL_QUEUE_DEVICE>(), CL_KERNEL_WORK_GROUP_SIZE, &maxNrThreads);
      globalWorkSize = cl::NDRange(maxNrThreads, ps.nrStations());
      localWorkSize  = cl::NDRange(maxNrThreads, 1);

      size_t nrSamples = ps.nrStations() * ps.nrSamplesPerChannel() * ps.nrChannelsPerSubband() * NR_POLARIZATIONS;
      nrOperations   = nrSamples * 2;
      nrBytesRead    = nrSamples * 2 * ps.nrBitsPerSample() / 8;
      nrBytesWritten = nrSamples * sizeof(std::complex<float>);
    }
};


class IncoherentStokesKernel : public Kernel
{
  public:
    IncoherentStokesKernel(const Parset &ps, cl::CommandQueue &queue, cl::Program &program, cl::Buffer &devIncoherentStokes, cl::Buffer &devInputSamples)
    :
      Kernel(ps, program, "incoherentStokes")
    {
      setArg(0, devIncoherentStokes);
      setArg(1, devInputSamples);

      unsigned nrTimes = ps.nrSamplesPerChannel() / ps.incoherentStokesTimeIntegrationFactor();
      size_t maxNrThreads;
      getWorkGroupInfo(queue.getInfo<CL_QUEUE_DEVICE>(), CL_KERNEL_WORK_GROUP_SIZE, &maxNrThreads);
      unsigned nrPasses = (nrTimes + maxNrThreads - 1) / maxNrThreads;
      unsigned nrTimesPerPass = (nrTimes + nrPasses - 1) / nrPasses;
      globalWorkSize = cl::NDRange(nrTimesPerPass * nrPasses, ps.nrChannelsPerSubband());
      localWorkSize  = cl::NDRange(nrTimesPerPass, 1);

      nrOperations   = ps.nrChannelsPerSubband() * ps.nrSamplesPerChannel() * ps.nrStations() * (ps.nrIncoherentStokes() == 1 ? 8 : 20 + 2.0 / ps.incoherentStokesTimeIntegrationFactor());
      nrBytesRead    = (size_t) ps.nrStations() * ps.nrChannelsPerSubband() * ps.nrSamplesPerChannel() * NR_POLARIZATIONS * sizeof(std::complex<float>);
      nrBytesWritten = (size_t) ps.nrIncoherentStokes() * nrTimes * ps.nrChannelsPerSubband() * sizeof(float);
    }
};


class BeamFormerKernel : public Kernel
{
  public:
    BeamFormerKernel(const Parset &ps, cl::Program &program, cl::Buffer &devComplexVoltages, cl::Buffer &devCorrectedData, cl::Buffer &devBeamFormerWeights)
    :
      Kernel(ps, program, "complexVoltages")
    {
      setArg(0, devComplexVoltages);
      setArg(1, devCorrectedData);
      setArg(2, devBeamFormerWeights);

      globalWorkSize = cl::NDRange(NR_POLARIZATIONS, ps.nrTABs(SAP), ps.nrChannelsPerSubband());
      localWorkSize  = cl::NDRange(NR_POLARIZATIONS, ps.nrTABs(SAP), 1);

      // FIXME: nrTABs
      //queue.enqueueNDRangeKernel(*this, cl::NullRange, cl::NDRange(16, ps.nrTABs(SAP), ps.nrChannelsPerSubband()), cl::NDRange(16, ps.nrTABs(SAP), 1), 0, &event);

      size_t count = ps.nrChannelsPerSubband() * ps.nrSamplesPerChannel() * NR_POLARIZATIONS;
      size_t nrWeightsBytes = ps.nrStations() * ps.nrTABs(SAP) * ps.nrChannelsPerSubband() * NR_POLARIZATIONS * sizeof(std::complex<float>);
      size_t nrSampleBytesPerPass = count * ps.nrStations() * sizeof(std::complex<float>);
      size_t nrComplexVoltagesBytesPerPass = count * ps.nrTABs(SAP) * sizeof(std::complex<float>);
      unsigned nrPasses = std::max((ps.nrStations() + 6) / 16, 1U);
      nrOperations   = count * ps.nrStations() * ps.nrTABs(SAP) * 8;
      nrBytesRead    = nrWeightsBytes + nrSampleBytesPerPass + (nrPasses - 1) * nrComplexVoltagesBytesPerPass;
      nrBytesWritten = nrPasses * nrComplexVoltagesBytesPerPass;
    }
};


class BeamFormerTransposeKernel : public Kernel
{
  public:
    BeamFormerTransposeKernel(const Parset &ps, cl::Program &program, cl::Buffer &devTransposedData, cl::Buffer &devComplexVoltages)
    :
      Kernel(ps, program, "transposeComplexVoltages")
    {
      ASSERT(ps.nrSamplesPerChannel() % 16 == 0);
      setArg(0, devTransposedData);
      setArg(1, devComplexVoltages);

      //globalWorkSize = cl::NDRange(256, (ps.nrTABs(SAP) + 15) / 16, (ps.nrChannelsPerSubband() + 15) / 16);
      globalWorkSize = cl::NDRange(256, (ps.nrTABs(SAP) + 15) / 16, ps.nrSamplesPerChannel() / 16);
      localWorkSize  = cl::NDRange(256, 1, 1);

      nrOperations   = 0;
      nrBytesRead    = (size_t) ps.nrChannelsPerSubband() * ps.nrSamplesPerChannel() * ps.nrTABs(SAP) * NR_POLARIZATIONS * sizeof(std::complex<float>),
      //nrBytesWritten = (size_t) ps.nrTABs(SAP) * NR_POLARIZATIONS * ps.nrSamplesPerChannel() * ps.nrChannelsPerSubband() * sizeof(std::complex<float>);
      nrBytesWritten = (size_t) ps.nrTABs(SAP) * NR_POLARIZATIONS * ps.nrChannelsPerSubband() * ps.nrSamplesPerChannel() * sizeof(std::complex<float>);
    }
};


#if 0
class Dedispersion_FFT_Kernel
{
  public:
    Dedispersion_FFT_Kernel(const Parset &ps, cl::Context &context, cl::Buffer &buffer)
    :
      ps(ps),
      plan(context, ps.dedispersionFFTsize()),
      buffer(buffer)
    {
      ASSERT(ps.nrSamplesPerChannel() % ps.dedispersionFFTsize() == 0);
    }

    void enqueue(cl::CommandQueue &queue, PerformanceCounter &counter, clFFT_Direction direction)
    {
      size_t nrFFTs = (size_t) ps.nrTABs(SAP) * NR_POLARIZATIONS * ps.nrChannelsPerSubband() * ps.nrSamplesPerChannel() / ps.dedispersionFFTsize();

      cl_int error = clFFT_ExecuteInterleaved(queue(), plan.plan, nrFFTs, direction, buffer(), buffer(), 0, 0, &event());

      if (error != CL_SUCCESS)
	throw cl::Error(error, "clFFT_ExecuteInterleaved");

      counter.doOperation(event,
	nrFFTs * 5 * ps.dedispersionFFTsize() * log2(ps.dedispersionFFTsize()),
	nrFFTs * ps.dedispersionFFTsize() * sizeof(std::complex<float>),
	nrFFTs * ps.dedispersionFFTsize() * sizeof(std::complex<float>));
    }

  private:
    const Parset &ps;
    FFT_Plan	 plan;
    cl::Buffer	 &buffer;
    cl::Event	 event;
};
#else
class DedispersionForwardFFTkernel : public FFT_Kernel
{
  public:
    DedispersionForwardFFTkernel(const Parset &ps, cl::Context &context, cl::Buffer &buffer)
    :
      FFT_Kernel(context, ps.dedispersionFFTsize(), ps.nrTABs(SAP) * NR_POLARIZATIONS * ps.nrChannelsPerSubband() * ps.nrSamplesPerChannel() / ps.dedispersionFFTsize(), true, buffer)
    {
      ASSERT(ps.nrSamplesPerChannel() % ps.dedispersionFFTsize() == 0);
    }
};


class DedispersionBackwardFFTkernel : public FFT_Kernel
{
  public:
    DedispersionBackwardFFTkernel(const Parset &ps, cl::Context &context, cl::Buffer &buffer)
    :
      FFT_Kernel(context, ps.dedispersionFFTsize(), ps.nrTABs(SAP) * NR_POLARIZATIONS * ps.nrChannelsPerSubband() * ps.nrSamplesPerChannel() / ps.dedispersionFFTsize(), false, buffer)
    {
      ASSERT(ps.nrSamplesPerChannel() % ps.dedispersionFFTsize() == 0);
    }
};
#endif


class DedispersionChirpKernel : public Kernel
{
  public:
    DedispersionChirpKernel(const Parset &ps, cl::Program &program, cl::CommandQueue &queue, cl::Buffer &buffer, cl::Buffer &DMs)
    :
      Kernel(ps, program, "applyChirp")
    {
      setArg(0, buffer);
      setArg(1, DMs);

      size_t maxNrThreads;
      getWorkGroupInfo(queue.getInfo<CL_QUEUE_DEVICE>(), CL_KERNEL_WORK_GROUP_SIZE, &maxNrThreads);
      unsigned fftSize = ps.dedispersionFFTsize();

      globalWorkSize = cl::NDRange(fftSize, ps.nrSamplesPerChannel() / fftSize, ps.nrChannelsPerSubband());
      //std::cout << "globalWorkSize = NDRange(" << fftSize << ", " << ps.nrSamplesPerChannel() / fftSize << ", " << ps.nrChannelsPerSubband() << ')' << std::endl;

      if (fftSize <= maxNrThreads) {
	localWorkSize = cl::NDRange(fftSize, 1, maxNrThreads / fftSize);
	//std::cout << "localWorkSize = NDRange(" << fftSize << ", 1, " << maxNrThreads / fftSize << ')' << std::endl;
      } else {
	unsigned divisor;

	for (divisor = 1; fftSize / divisor > maxNrThreads || fftSize % divisor != 0; divisor ++)
	  ;

	localWorkSize = cl::NDRange(fftSize / divisor, 1, 1);
	//std::cout << "localWorkSize = NDRange(" << fftSize / divisor << ", 1, 1))" << std::endl;
      }

      nrOperations = (size_t) NR_POLARIZATIONS * ps.nrChannelsPerSubband() * ps.nrSamplesPerChannel() * (9 * ps.nrTABs(SAP) + 17),
      nrBytesRead  = nrBytesWritten = sizeof(std::complex<float>) * ps.nrTABs(SAP) * NR_POLARIZATIONS * ps.nrChannelsPerSubband() * ps.nrSamplesPerChannel();
    }

    void enqueue(cl::CommandQueue &queue, PerformanceCounter &counter, double subbandFrequency)
    {
      setArg(2, (float) subbandFrequency);
      Kernel::enqueue(queue, counter);
    }
};


class CoherentStokesKernel : public Kernel
{
  public:
    CoherentStokesKernel(const Parset &ps, cl::Program &program, cl::Buffer &devStokesData, cl::Buffer &devComplexVoltages)
    :
      Kernel(ps, program, "coherentStokes")
    {
      ASSERT(ps.nrChannelsPerSubband() >= 16 && ps.nrChannelsPerSubband() % 16 == 0);
      ASSERT(ps.nrCoherentStokes() == 1 || ps.nrCoherentStokes() == 4);
      setArg(0, devStokesData);
      setArg(1, devComplexVoltages);

      globalWorkSize = cl::NDRange(256, (ps.nrTABs(SAP) + 15) / 16, (ps.nrChannelsPerSubband() + 15) / 16);
      localWorkSize  = cl::NDRange(256, 1, 1);

      nrOperations   = (size_t) ps.nrChannelsPerSubband() * ps.nrSamplesPerChannel() * ps.nrTABs(SAP) * (ps.nrCoherentStokes() == 1 ? 8 : 20 + 2.0 / ps.coherentStokesTimeIntegrationFactor());
      nrBytesRead    = (size_t) ps.nrChannelsPerSubband() * ps.nrSamplesPerChannel() * ps.nrTABs(SAP) * NR_POLARIZATIONS * sizeof(std::complex<float>);
      nrBytesWritten = (size_t) ps.nrTABs(SAP) * ps.nrCoherentStokes() * ps.nrSamplesPerChannel() / ps.coherentStokesTimeIntegrationFactor() * ps.nrChannelsPerSubband() * sizeof(float);
    }
};


class UHEP_BeamFormerKernel : public Kernel
{
  public:
    UHEP_BeamFormerKernel(const Parset &ps, cl::Program &program, cl::Buffer &devComplexVoltages, cl::Buffer &devInputSamples, cl::Buffer &devBeamFormerWeights)
    :
      Kernel(ps, program, "complexVoltages")
    {
      setArg(0, devComplexVoltages);
      setArg(1, devInputSamples);
      setArg(2, devBeamFormerWeights);

#if 1
      globalWorkSize = cl::NDRange(NR_POLARIZATIONS, ps.nrTABs(SAP), ps.nrSubbands());
      localWorkSize  = cl::NDRange(NR_POLARIZATIONS, ps.nrTABs(SAP), 1);

      size_t count = ps.nrSubbands() * (ps.nrSamplesPerChannel() + NR_STATION_FILTER_TAPS - 1) * NR_POLARIZATIONS;
      size_t nrWeightsBytes = ps.nrStations() * ps.nrTABs(SAP) * ps.nrSubbands() * NR_POLARIZATIONS * sizeof(std::complex<float>);
      size_t nrSampleBytes = count * ps.nrStations() * ps.nrBytesPerComplexSample();
      size_t nrComplexVoltagesBytesPerPass = count * ps.nrTABs(SAP) * sizeof(std::complex<float>);
      unsigned nrPasses = std::max((ps.nrStations() + 6) / 16, 1U);
      nrOperations   = count * ps.nrStations() * ps.nrTABs(SAP) * 8;
      nrBytesRead    = nrWeightsBytes + nrSampleBytes + (nrPasses - 1) * nrComplexVoltagesBytesPerPass;
      nrBytesWritten = nrPasses * nrComplexVoltagesBytesPerPass;
#else
      ASSERT(ps.nrTABs(SAP) % 3 == 0);
      ASSERT(ps.nrStations() % 6 == 0);
      unsigned nrThreads = NR_POLARIZATIONS * (ps.nrTABs(SAP) / 3) * (ps.nrStations() / 6);
      globalWorkSize = cl::NDRange(nrThreads, ps.nrSubbands());
      localWorkSize  = cl::NDRange(nrThreads, 1);
      //globalWorkSize = cl::NDRange(ps.nrStations() / 6, ps.nrTABs(SAP) / 3, ps.nrSubbands());
      //localWorkSize  = cl::NDRange(ps.nrStations() / 6, ps.nrTABs(SAP) / 3, 1);

      size_t count = ps.nrSubbands() * (ps.nrSamplesPerChannel() + NR_STATION_FILTER_TAPS - 1) * NR_POLARIZATIONS;
      size_t nrWeightsBytes = ps.nrStations() * ps.nrTABs(SAP) * ps.nrSubbands() * NR_POLARIZATIONS * sizeof(std::complex<float>);
      size_t nrSampleBytes = count * ps.nrStations() * ps.nrBytesPerComplexSample();
      size_t nrComplexVoltagesBytes = count * ps.nrTABs(SAP) * sizeof(std::complex<float>);
      nrOperations   = count * ps.nrStations() * ps.nrTABs(SAP) * 8;
      nrBytesRead    = nrWeightsBytes + nrSampleBytes;
      nrBytesWritten = nrComplexVoltagesBytes;
#endif
    }
};


class UHEP_TransposeKernel : public Kernel
{
  public:
    UHEP_TransposeKernel(const Parset &ps, cl::Program &program, cl::Buffer &devFFTedData, cl::Buffer &devComplexVoltages, cl::Buffer &devReverseSubbandMapping)
    :
      Kernel(ps, program, "UHEP_Transpose")
    {
      setArg(0, devFFTedData);
      setArg(1, devComplexVoltages);
      setArg(2, devReverseSubbandMapping);

      globalWorkSize = cl::NDRange(256, (ps.nrTABs(SAP) + 15) / 16, 512 / 16);
      localWorkSize  = cl::NDRange(256, 1, 1);

      nrOperations   = 0;
      nrBytesRead    = (size_t) ps.nrSubbands() * (ps.nrSamplesPerChannel() + NR_STATION_FILTER_TAPS - 1) * ps.nrTABs(SAP) * NR_POLARIZATIONS * sizeof(std::complex<float>);
      nrBytesWritten = (size_t) ps.nrTABs(SAP) * NR_POLARIZATIONS * (ps.nrSamplesPerChannel() + NR_STATION_FILTER_TAPS - 1) * 512 * sizeof(std::complex<float>);
    }
};


class UHEP_InvFFT_Kernel : public Kernel
{
  public:
    UHEP_InvFFT_Kernel(const Parset &ps, cl::Program &program, cl::Buffer &devFFTedData)
    :
      Kernel(ps, program, "inv_fft")
    {
      setArg(0, devFFTedData);
      setArg(1, devFFTedData);

      globalWorkSize = cl::NDRange(128, ps.nrTABs(SAP) * NR_POLARIZATIONS * ps.nrSamplesPerChannel());
      localWorkSize  = cl::NDRange(128, 1);

      size_t nrFFTs = (size_t) ps.nrTABs(SAP) * NR_POLARIZATIONS * (ps.nrSamplesPerChannel() + NR_STATION_FILTER_TAPS - 1);
      nrOperations   = nrFFTs * 5 * 1024 * 10;
      nrBytesRead    = nrFFTs * 512 * sizeof(std::complex<float>);
      nrBytesWritten = nrFFTs * 1024 * sizeof(float);
    }
};


class UHEP_InvFIR_Kernel : public Kernel
{
  public:
    UHEP_InvFIR_Kernel(const Parset &ps, cl::CommandQueue &queue, cl::Program &program, cl::Buffer &devInvFIRfilteredData, cl::Buffer &devFFTedData, cl::Buffer &devInvFIRfilterWeights)
    :
      Kernel(ps, program, "invFIRfilter")
    {
      setArg(0, devInvFIRfilteredData);
      setArg(1, devFFTedData);
      setArg(2, devInvFIRfilterWeights);

      size_t maxNrThreads, nrThreads;
      getWorkGroupInfo(queue.getInfo<CL_QUEUE_DEVICE>(), CL_KERNEL_WORK_GROUP_SIZE, &maxNrThreads);
      // round down to nearest power of two
      for (nrThreads = 1024; nrThreads > maxNrThreads; nrThreads /= 2)
	;

      globalWorkSize = cl::NDRange(1024, NR_POLARIZATIONS, ps.nrTABs(SAP));
      localWorkSize  = cl::NDRange(nrThreads, 1, 1);

      size_t count = ps.nrTABs(SAP) * NR_POLARIZATIONS * 1024;
      nrOperations   = count * ps.nrSamplesPerChannel() * NR_STATION_FILTER_TAPS * 2;
      nrBytesRead    = count * (ps.nrSamplesPerChannel() + NR_STATION_FILTER_TAPS - 1) * sizeof(float);
      nrBytesWritten = count * ps.nrSamplesPerChannel() * sizeof(float);
    }
};


class UHEP_TriggerKernel : public Kernel
{
  public:
    UHEP_TriggerKernel(const Parset &ps, cl::Program &program, cl::Buffer &devTriggerInfo, cl::Buffer &devInvFIRfilteredData)
    :
      Kernel(ps, program, "trigger")
    {
      setArg(0, devTriggerInfo);
      setArg(1, devInvFIRfilteredData);

      globalWorkSize = cl::NDRange(16, 16, ps.nrTABs(SAP));
      localWorkSize  = cl::NDRange(16, 16, 1);

      nrOperations   = (size_t) ps.nrTABs(SAP) * ps.nrSamplesPerChannel() * 1024 * (3 /* power */ + 2 /* window */ + 1 /* max */ + 7 /* mean/variance */);
      nrBytesRead    = (size_t) ps.nrTABs(SAP) * NR_POLARIZATIONS * ps.nrSamplesPerChannel() * 1024 * sizeof(float);
      nrBytesWritten = (size_t) ps.nrTABs(SAP) * sizeof(TriggerInfo);
    }
};


WorkQueue::WorkQueue(Pipeline &pipeline)
:
  gpu(omp_get_thread_num() % nrGPUs),
  device(pipeline.devices[gpu]),
  ps(pipeline.ps)
{
#if defined __linux__ && defined USE_B7015
  set_affinity(gpu);
#endif

  queue = cl::CommandQueue(pipeline.context, device, profiling ? CL_QUEUE_PROFILING_ENABLE : 0);
}


CorrelatorWorkQueue::CorrelatorWorkQueue(CorrelatorPipeline &pipeline)
:
  WorkQueue(pipeline),
  pipeline(pipeline),
  devBufferA(pipeline.context, CL_MEM_READ_WRITE, ps.nrStations() * NR_POLARIZATIONS * ps.nrSamplesPerChannel() * ps.nrChannelsPerSubband() * sizeof(std::complex<float>)),
  devBufferB(pipeline.context, CL_MEM_READ_WRITE, ps.nrStations() * NR_POLARIZATIONS * ps.nrSamplesPerChannel() * ps.nrChannelsPerSubband() * sizeof(std::complex<float>)),
  bandPassCorrectionWeights(boost::extents[ps.nrChannelsPerSubband()], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY),
  delaysAtBegin(boost::extents[ps.nrBeams()][ps.nrStations()][NR_POLARIZATIONS], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY),
  delaysAfterEnd(boost::extents[ps.nrBeams()][ps.nrStations()][NR_POLARIZATIONS], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY),
  phaseOffsets(boost::extents[ps.nrBeams()][NR_POLARIZATIONS], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY),
  //inputSamples(boost::extents[ps.nrStations()][(ps.nrSamplesPerChannel() + NR_TAPS - 1) * ps.nrChannelsPerSubband()][NR_POLARIZATIONS][ps.nrBytesPerComplexSample()], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY),
 //visibilities(boost::extents[ps.nrBaselines()][ps.nrChannelsPerSubband()][NR_POLARIZATIONS][NR_POLARIZATIONS], queue, CL_MEM_READ_ONLY, CL_MEM_WRITE_ONLY)
  inputSamples(boost::extents[ps.nrStations()][(ps.nrSamplesPerChannel() + NR_TAPS - 1) * ps.nrChannelsPerSubband()][NR_POLARIZATIONS][ps.nrBytesPerComplexSample()], queue, CL_MEM_WRITE_ONLY, devBufferA),
 visibilities(boost::extents[ps.nrBaselines()][ps.nrChannelsPerSubband()][NR_POLARIZATIONS][NR_POLARIZATIONS], queue, CL_MEM_READ_ONLY, devBufferB)
{
  size_t firWeightsSize = ps.nrChannelsPerSubband() * NR_TAPS * sizeof(float);
  devFIRweights = cl::Buffer(pipeline.context, CL_MEM_READ_ONLY, firWeightsSize);
  queue.enqueueWriteBuffer(devFIRweights, CL_TRUE, 0, ps.nrChannelsPerSubband() * NR_TAPS * sizeof(float), pipeline.filterBank.getWeights().origin());

#if 0
  size_t filteredDataSize = ps.nrStations() * NR_POLARIZATIONS * ps.nrSamplesPerChannel() * ps.nrChannelsPerSubband() * sizeof(std::complex<float>);
  devFilteredData = cl::Buffer(pipeline.context, CL_MEM_READ_WRITE, filteredDataSize);
  devCorrectedData = cl::Buffer(pipeline.context, CL_MEM_READ_WRITE, filteredDataSize);
#else
  devFilteredData = devBufferB;
  devCorrectedData = devBufferA;
#endif

  if (ps.correctBandPass()) {
    BandPass::computeCorrectionFactors(bandPassCorrectionWeights.origin(), ps.nrChannelsPerSubband());
    bandPassCorrectionWeights.hostToDevice(CL_TRUE);
  }
}


void CorrelatorWorkQueue::doWork()
{
  FIR_FilterKernel firFilterKernel(ps, queue, pipeline.firFilterProgram, devFilteredData, inputSamples, devFIRweights);
  Filter_FFT_Kernel fftKernel(ps, pipeline.context, devFilteredData);
  DelayAndBandPassKernel delayAndBandPassKernel(ps, pipeline.delayAndBandPassProgram, devCorrectedData, devFilteredData, delaysAtBegin, delaysAfterEnd, phaseOffsets, bandPassCorrectionWeights);
  CorrelatorKernel correlatorKernel(ps, queue, pipeline.correlatorProgram, visibilities, devCorrectedData);
  double startTime = ps.startTime(), currentTime, stopTime = ps.stopTime(), blockTime = ps.CNintegrationTime();

#pragma omp barrier

  double executionStartTime = getTime();

  for (unsigned block = 0; (currentTime = startTime + block * blockTime) < stopTime; block ++) {
#pragma omp single
#pragma omp critical (cout)
    std::cout << "block = " << block << ", time = " << to_simple_string(from_ustime_t(currentTime)) << std::endl;

    memset(delaysAtBegin.origin(), 0, delaysAtBegin.bytesize());
    memset(delaysAfterEnd.origin(), 0, delaysAfterEnd.bytesize());
    memset(phaseOffsets.origin(), 0, phaseOffsets.bytesize());

    // FIXME!!!
    if (ps.nrStations() >= 3)
      delaysAtBegin[0][2][0] = 1e-6, delaysAfterEnd[0][2][0] = 1.1e-6;

    delaysAtBegin.hostToDevice(CL_FALSE);
    delaysAfterEnd.hostToDevice(CL_FALSE);
    phaseOffsets.hostToDevice(CL_FALSE);
    queue.finish();

#pragma omp barrier

#pragma omp for schedule(dynamic)
    for (unsigned subband = 0; subband < ps.nrSubbands(); subband ++) {
      try {
#if defined USE_TEST_DATA
	if (subband == 0)
	  setTestPattern();
#endif

	{
#if defined USE_B7015
	  OMP_ScopedLock scopedLock(pipeline.hostToDeviceLock[gpu / 2]);
#endif
	  inputSamples.hostToDevice(CL_TRUE);
	  pipeline.samplesCounter.doOperation(inputSamples.event, 0, 0, inputSamples.bytesize());
	}

	if (ps.nrChannelsPerSubband() > 1) {
	  firFilterKernel.enqueue(queue, pipeline.firFilterCounter);
	  fftKernel.enqueue(queue, pipeline.fftCounter);
	}

	delayAndBandPassKernel.enqueue(queue, pipeline.delayAndBandPassCounter, subband);
	correlatorKernel.enqueue(queue, pipeline.correlatorCounter);
	queue.finish();

	{
#if defined USE_B7015
	  OMP_ScopedLock scopedLock(pipeline.deviceToHostLock[gpu / 2]);
#endif
	  visibilities.deviceToHost(CL_TRUE);
	  pipeline.visibilitiesCounter.doOperation(visibilities.event, 0, visibilities.bytesize(), 0);
	}

#if defined USE_TEST_DATA
	if (subband == 0)
	  printTestOutput();
#endif
      } catch (cl::Error &error) {
#pragma omp critical (cerr)
	std::cerr << "OpenCL error: " << error.what() << ": " << errorMessage(error.err()) << std::endl << error;
	exit(1);
      }
    }
  }

#pragma omp barrier

#pragma omp master
  if (!profiling)
#pragma omp critical (cout)
    std::cout << "run time = " << getTime() - executionStartTime << std::endl;
}


// complexVoltages()
// float2 (*ComplexVoltagesType)[NR_CHANNELS][NR_TIMES_PER_BLOCK][NR_TABS][NR_POLARIZATIONS];
// transpose()
//
// float2 (*DedispersedDataType)[nrTABs][NR_POLARIZATIONS][ps.nrChannelsPerSubband()][ps.nrSamplesPerChannel()];
// FFT()
//
// applyChrip()
//
// FFT-1()
// float2 (*DedispersedDataType)[nrTABs][NR_POLARIZATIONS][ps.nrChannelsPerSubband()][ps.nrSamplesPerChannel()];
// (*ComplexVoltagesType)[NR_CHANNELS][NR_TIMES_PER_BLOCK][NR_TABS];
// computeStokes()
// float (*StokesType)[NR_TABS][NR_STOKES][NR_TIMES_PER_BLOCK / STOKES_INTEGRATION_SAMPLES][NR_CHANNELS];


BeamFormerWorkQueue::BeamFormerWorkQueue(BeamFormerPipeline &pipeline)
:
  WorkQueue(pipeline),
  pipeline(pipeline),
  inputSamples(boost::extents[ps.nrStations()][ps.nrSamplesPerChannel() * ps.nrChannelsPerSubband()][NR_POLARIZATIONS][ps.nrBytesPerComplexSample()], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY),
  devFilteredData(pipeline.context, CL_MEM_READ_WRITE, ps.nrStations() * NR_POLARIZATIONS * ps.nrSamplesPerChannel() * ps.nrChannelsPerSubband() * sizeof(std::complex<float>)),
  bandPassCorrectionWeights(boost::extents[ps.nrChannelsPerSubband()], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY),
  delaysAtBegin(boost::extents[ps.nrBeams()][ps.nrStations()][NR_POLARIZATIONS], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY),
  delaysAfterEnd(boost::extents[ps.nrBeams()][ps.nrStations()][NR_POLARIZATIONS], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY),
  phaseOffsets(boost::extents[ps.nrBeams()][NR_POLARIZATIONS], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY),
  devCorrectedData(cl::Buffer(pipeline.context, CL_MEM_READ_WRITE, ps.nrStations() * ps.nrChannelsPerSubband() * ps.nrSamplesPerChannel() * NR_POLARIZATIONS * sizeof(std::complex<float>))),
  beamFormerWeights(boost::extents[ps.nrStations()][ps.nrChannelsPerSubband()][ps.nrTABs(SAP)], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY),
  devComplexVoltages(cl::Buffer(pipeline.context, CL_MEM_READ_WRITE, ps.nrChannelsPerSubband() * ps.nrSamplesPerChannel() * ps.nrTABs(SAP) * NR_POLARIZATIONS * sizeof(std::complex<float>))),
  //transposedComplexVoltages(boost::extents[ps.nrTABs(SAP)][NR_POLARIZATIONS][ps.nrSamplesPerChannel()][ps.nrChannelsPerSubband()], queue, CL_MEM_READ_ONLY, CL_MEM_READ_WRITE)
  transposedComplexVoltages(boost::extents[ps.nrTABs(SAP)][NR_POLARIZATIONS][ps.nrChannelsPerSubband()][ps.nrSamplesPerChannel()], queue, CL_MEM_READ_ONLY, CL_MEM_READ_WRITE),
  DMs(boost::extents[ps.nrTABs(SAP)], queue, CL_MEM_READ_ONLY, CL_MEM_WRITE_ONLY)
{
  if (ps.correctBandPass()) {
    BandPass::computeCorrectionFactors(bandPassCorrectionWeights.origin(), ps.nrChannelsPerSubband());
    bandPassCorrectionWeights.hostToDevice(CL_TRUE);
  }
}


void BeamFormerWorkQueue::doWork()
{
  //queue.enqueueWriteBuffer(devFIRweights, CL_TRUE, 0, firWeightsSize, firFilterWeights);
  bandPassCorrectionWeights.hostToDevice(CL_TRUE);
  DMs.hostToDevice(CL_TRUE);

  IntToFloatKernel intToFloatKernel(ps, queue, pipeline.intToFloatProgram, devFilteredData, inputSamples);
  Filter_FFT_Kernel fftKernel(ps, pipeline.context, devFilteredData);
  DelayAndBandPassKernel delayAndBandPassKernel(ps, pipeline.delayAndBandPassProgram, devCorrectedData, devFilteredData, delaysAtBegin, delaysAfterEnd, phaseOffsets, bandPassCorrectionWeights);
  BeamFormerKernel beamFormerKernel(ps, pipeline.beamFormerProgram, devComplexVoltages, devCorrectedData, beamFormerWeights);
  BeamFormerTransposeKernel transposeKernel(ps, pipeline.transposeProgram, transposedComplexVoltages, devComplexVoltages);
  DedispersionForwardFFTkernel dedispersionForwardFFTkernel(ps, pipeline.context, transposedComplexVoltages);
  DedispersionBackwardFFTkernel dedispersionBackwardFFTkernel(ps, pipeline.context, transposedComplexVoltages);
  DedispersionChirpKernel dedispersionChirpKernel(ps, pipeline.dedispersionChirpProgram, queue, transposedComplexVoltages, DMs);
  double startTime = ps.startTime(), currentTime, stopTime = ps.stopTime(), blockTime = ps.CNintegrationTime();

#pragma omp barrier

  double executionStartTime = getTime();

  for (unsigned block = 0; (currentTime = startTime + block * blockTime) < stopTime; block ++) {
#pragma omp single
#pragma omp critical (cout)
    std::cout << "block = " << block << ", time = " << to_simple_string(from_ustime_t(currentTime)) << std::endl;

    memset(delaysAtBegin.origin(), 0, delaysAtBegin.bytesize());
    memset(delaysAfterEnd.origin(), 0, delaysAfterEnd.bytesize());
    memset(phaseOffsets.origin(), 0, phaseOffsets.bytesize());

    // FIXME!!!
    if (ps.nrStations() >= 3)
      delaysAtBegin[0][2][0] = 1e-6, delaysAfterEnd[0][2][0] = 1.1e-6;

    delaysAtBegin.hostToDevice(CL_FALSE);
    delaysAfterEnd.hostToDevice(CL_FALSE);
    phaseOffsets.hostToDevice(CL_FALSE);
    beamFormerWeights.hostToDevice(CL_FALSE);
    queue.finish();

#pragma omp barrier

#pragma omp for schedule(dynamic)
    for (unsigned subband = 0; subband < ps.nrSubbands(); subband ++) {
      try {
#if 1
	{
#if defined USE_B7015
	  OMP_ScopedLock scopedLock(pipeline.hostToDeviceLock[gpu / 2]);
#endif
	  inputSamples.hostToDevice(CL_TRUE);
	  pipeline.samplesCounter.doOperation(inputSamples.event, 0, 0, inputSamples.bytesize());
	}
#endif

//#pragma omp critical (GPU)
{
	if (ps.nrChannelsPerSubband() > 1) {
	  intToFloatKernel.enqueue(queue, pipeline.intToFloatCounter);
	  fftKernel.enqueue(queue, pipeline.fftCounter);
	}

	delayAndBandPassKernel.enqueue(queue, pipeline.delayAndBandPassCounter, subband);
	beamFormerKernel.enqueue(queue, pipeline.beamFormerCounter);
	transposeKernel.enqueue(queue, pipeline.transposeCounter);
	dedispersionForwardFFTkernel.enqueue(queue, pipeline.dedispersionForwardFFTcounter);
	dedispersionChirpKernel.enqueue(queue, pipeline.dedispersionChirpCounter, ps.subbandToFrequencyMapping()[subband]);
	dedispersionBackwardFFTkernel.enqueue(queue, pipeline.dedispersionBackwardFFTcounter);

	queue.finish();
}

	//queue.enqueueReadBuffer(devComplexVoltages, CL_TRUE, 0, hostComplexVoltages.bytesize(), hostComplexVoltages.origin());
	//dedispersedData.deviceToHost(CL_TRUE);
      } catch (cl::Error &error) {
#pragma omp critical (cerr)
	std::cerr << "OpenCL error: " << error.what() << ": " << errorMessage(error.err()) << std::endl << error;
	exit(1);
      }
    }
  }

#pragma omp barrier

#pragma omp master
  if (!profiling)
#pragma omp critical (cout)
    std::cout << "run time = " << getTime() - executionStartTime << std::endl;
}


UHEP_WorkQueue::UHEP_WorkQueue(UHEP_Pipeline &pipeline)
:
  WorkQueue(pipeline),
  pipeline(pipeline),
  hostInputSamples(boost::extents[ps.nrStations()][ps.nrSubbands()][ps.nrSamplesPerChannel() + NR_STATION_FILTER_TAPS - 1][NR_POLARIZATIONS][ps.nrBytesPerComplexSample()], queue, CL_MEM_WRITE_ONLY),
  hostBeamFormerWeights(boost::extents[ps.nrStations()][ps.nrSubbands()][ps.nrTABs(SAP)], queue, CL_MEM_WRITE_ONLY),
  hostTriggerInfo(ps.nrTABs(SAP), queue, CL_MEM_READ_ONLY)
{
  size_t inputSamplesSize = ps.nrStations() * ps.nrSubbands() * (ps.nrSamplesPerChannel() + NR_STATION_FILTER_TAPS - 1) * NR_POLARIZATIONS * ps.nrBytesPerComplexSample();
  size_t complexVoltagesSize = ps.nrSubbands() * (ps.nrSamplesPerChannel() + NR_STATION_FILTER_TAPS - 1) * ps.nrTABs(SAP) * NR_POLARIZATIONS * sizeof(std::complex<float>);
  size_t transposedDataSize = ps.nrTABs(SAP) * NR_POLARIZATIONS * (ps.nrSamplesPerChannel() + NR_STATION_FILTER_TAPS - 1) * 512 * sizeof(std::complex<float>);
  size_t invFIRfilteredDataSize = ps.nrTABs(SAP) * NR_POLARIZATIONS * ps.nrSamplesPerChannel() * 512 * sizeof(std::complex<float>);

  size_t buffer0size = std::max(inputSamplesSize, transposedDataSize);
  size_t buffer1size = std::max(complexVoltagesSize, invFIRfilteredDataSize);

  devBuffers[0] = cl::Buffer(pipeline.context, CL_MEM_READ_WRITE, buffer0size);
  devBuffers[1] = cl::Buffer(pipeline.context, CL_MEM_READ_WRITE, buffer1size);

  size_t beamFormerWeightsSize = ps.nrStations() * ps.nrSubbands() * ps.nrTABs(SAP) * sizeof(std::complex<float>);
  devBeamFormerWeights = cl::Buffer(pipeline.context, CL_MEM_READ_ONLY, beamFormerWeightsSize);

  devInputSamples = devBuffers[0];
  devComplexVoltages = devBuffers[1];

  devReverseSubbandMapping = cl::Buffer(pipeline.context, CL_MEM_READ_ONLY, 512 * sizeof(int));
  devInvFIRfilterWeights = cl::Buffer(pipeline.context, CL_MEM_READ_ONLY, 1024 * NR_STATION_FILTER_TAPS * sizeof(float));
  devFFTedData = devBuffers[0];
  devInvFIRfilteredData = devBuffers[1];

  devTriggerInfo = cl::Buffer(pipeline.context, CL_MEM_WRITE_ONLY, ps.nrTABs(SAP) * sizeof(TriggerInfo));
}


void UHEP_WorkQueue::doWork(const float * /*delaysAtBegin*/, const float * /*delaysAfterEnd*/, const float * /*phaseOffsets*/)
{
  UHEP_BeamFormerKernel beamFormer(ps, pipeline.beamFormerProgram, devComplexVoltages, devInputSamples, devBeamFormerWeights);
  UHEP_TransposeKernel  transpose(ps, pipeline.transposeProgram, devFFTedData, devComplexVoltages, devReverseSubbandMapping);
  UHEP_InvFFT_Kernel	invFFT(ps, pipeline.invFFTprogram, devFFTedData);
  UHEP_InvFIR_Kernel	invFIR(ps, queue, pipeline.invFIRfilterProgram, devInvFIRfilteredData, devFFTedData, devInvFIRfilterWeights);
  UHEP_TriggerKernel	trigger(ps, pipeline.triggerProgram, devTriggerInfo, devInvFIRfilteredData);
  double startTime = ps.startTime(), stopTime = ps.stopTime(), blockTime = ps.CNintegrationTime();
  unsigned nrBlocks = (stopTime - startTime) / blockTime;

  queue.enqueueWriteBuffer(devInvFIRfilterWeights, CL_FALSE, 0, sizeof invertedStationPPFWeights, invertedStationPPFWeights);
  queue.enqueueWriteBuffer(devReverseSubbandMapping, CL_TRUE, 0, 512 * sizeof(int), reverseSubbandMapping);

#pragma omp barrier

  double executionStartTime = getTime();

#pragma omp for schedule(dynamic)
  for (unsigned block = 0; block < nrBlocks; block ++) {
    try {
      double currentTime = startTime + block * blockTime;

//#pragma omp single // FIXME: why does the compiler complain here???
#pragma omp critical (cout)
      std::cout << "block = " << block << ", time = " << to_simple_string(from_ustime_t(currentTime)) << std::endl;

#if 0
      {
#if defined USE_B7015
	OMP_ScopedLock scopedLock(pipeline.hostToDeviceLock[gpu / 2]);
#endif
	queue.enqueueWriteBuffer(devInputSamples, CL_TRUE, 0, sampledDataSize, hostInputSamples.origin(), 0, &samplesEvent);
      }
#endif

      queue.enqueueWriteBuffer(devBeamFormerWeights, CL_FALSE, 0, hostBeamFormerWeights.bytesize(), hostBeamFormerWeights.origin(), 0, &beamFormerWeightsEvent);
      pipeline.beamFormerWeightsCounter.doOperation(beamFormerWeightsEvent, 0, 0, hostBeamFormerWeights.bytesize());

      queue.enqueueWriteBuffer(devInputSamples, CL_FALSE, 0, hostInputSamples.bytesize(), hostInputSamples.origin(), 0, &inputSamplesEvent);
      pipeline.samplesCounter.doOperation(inputSamplesEvent, 0, 0, hostInputSamples.bytesize());

      beamFormer.enqueue(queue, pipeline.beamFormerCounter);
      transpose.enqueue(queue, pipeline.transposeCounter);
      invFFT.enqueue(queue, pipeline.invFFTcounter);
      invFIR.enqueue(queue, pipeline.invFIRfilterCounter);
      trigger.enqueue(queue, pipeline.triggerCounter);
      queue.finish(); // necessary to overlap I/O & computations ???
      queue.enqueueReadBuffer(devTriggerInfo, CL_TRUE, 0, hostTriggerInfo.size() * sizeof(TriggerInfo), &hostTriggerInfo[0]);
    } catch (cl::Error &error) {
#pragma omp critical (cerr)
      std::cerr << "OpenCL error: " << error.what() << ": " << errorMessage(error.err()) << std::endl << error;
      exit(1);
    }
  }

#pragma omp barrier

#pragma omp master
  if (!profiling)
#pragma omp critical (cout)
    std::cout << "run time = " << getTime() - executionStartTime << std::endl;
}


#if defined USE_TEST_DATA

void CorrelatorWorkQueue::setTestPattern()
{
  if (ps.nrStations() >= 3) {
    double centerFrequency = 384 * ps.sampleRate();
    double baseFrequency = centerFrequency - .5 * ps.sampleRate();
    unsigned testSignalChannel = ps.nrChannelsPerSubband() >= 231 ? 230 : ps.nrChannelsPerSubband() / 2;
    double signalFrequency = baseFrequency + testSignalChannel * ps.sampleRate() / ps.nrChannelsPerSubband();

    for (unsigned time = 0; time < (NR_TAPS - 1 + ps.nrSamplesPerChannel()) * ps.nrChannelsPerSubband(); time ++) {
      double phi = 2.0 * M_PI * signalFrequency * time / ps.sampleRate();

      switch (ps.nrBytesPerComplexSample()) {
	case 4 : reinterpret_cast<std::complex<short> &>(hostInputSamples[2][time][1][0]) = std::complex<short>((short) rint(32767 * cos(phi)), (short) rint(32767 * sin(phi)));
		 break;

	case 2 : reinterpret_cast<std::complex<signed char> &>(hostInputSamples[2][time][1][0]) = std::complex<signed char>((signed char) rint(127 * cos(phi)), (signed char) rint(127 * sin(phi)));
		 break;
      }
    }
  }
}


void CorrelatorWorkQueue::printTestOutput()
{
  if (ps.nrBaselines() >= 6)
#pragma omp critical (cout)
  {
    std::cout << "newgraph newcurve linetype solid pts" << std::endl;

    //for (int channel = 0; channel < ps.nrChannelsPerSubband(); channel ++)
    if (ps.nrChannelsPerSubband() == 256)
      for (int channel = 228; channel <= 232; channel ++)
	std::cout << channel << ' ' << hostVisibilities[5][channel][1][1] << std::endl;
  }
}

#endif


void CorrelatorPipeline::doWork()
{
#pragma omp parallel num_threads((profiling ? 1 : 2) * nrGPUs)
  try
  {
    CorrelatorWorkQueue(*this).doWork();
  } catch (cl::Error &error) {
#pragma omp critical (cerr)
    std::cerr << "OpenCL error: " << error.what() << ": " << errorMessage(error.err()) << std::endl << error;
    exit(1);
  }
}


void BeamFormerPipeline::doWork()
{
#pragma omp parallel num_threads((profiling ? 1 : 2) * nrGPUs)
  try
  {
    BeamFormerWorkQueue(*this).doWork();
  } catch (cl::Error &error) {
#pragma omp critical (cerr)
    std::cerr << "OpenCL error: " << error.what() << ": " << errorMessage(error.err()) << std::endl << error;
    exit(1);
  }
}


void UHEP_Pipeline::doWork()
{
  float delaysAtBegin[ps.nrBeams()][ps.nrStations()][NR_POLARIZATIONS] __attribute__((aligned(32)));
  float delaysAfterEnd[ps.nrBeams()][ps.nrStations()][NR_POLARIZATIONS] __attribute__((aligned(32)));
  float phaseOffsets[ps.nrStations()][NR_POLARIZATIONS] __attribute__((aligned(32)));

  memset(delaysAtBegin, 0, sizeof delaysAtBegin);
  memset(delaysAfterEnd, 0, sizeof delaysAfterEnd);
  memset(phaseOffsets, 0, sizeof phaseOffsets);
  delaysAtBegin[0][2][0] = 1e-6, delaysAfterEnd[0][2][0] = 1.1e-6;

#pragma omp parallel num_threads((profiling ? 1 : 2) * nrGPUs)
  try
  {
      UHEP_WorkQueue(*this).doWork(&delaysAtBegin[0][0][0], &delaysAfterEnd[0][0][0], &phaseOffsets[0][0]);
  } catch (cl::Error &error) {
#pragma omp critical (cerr)
    std::cerr << "OpenCL error: " << error.what() << ": " << errorMessage(error.err()) << std::endl << error;
    exit(1);
  }
}


class UnitTest
{
  protected:
    UnitTest(const Parset &ps, const char *programName = 0)
    :
      counter(programName != 0 ? programName : "test")
    {
      createContext(context, devices);
      queue = cl::CommandQueue(context, devices[0], CL_QUEUE_PROFILING_ENABLE);

      if (programName != 0)
	program = createProgram(ps, context, devices, programName);
    }

    template <typename T> void check(T actual, T expected)
    {
      if (expected != actual) {
	std::cerr << "Test FAILED: expected " << expected << ", computed " << actual << std::endl;
	exit(1);
      } else {
	std::cout << "Test OK" << std::endl;
      }
    }

    cl::Context context;
    std::vector<cl::Device> devices;
    cl::Program program;
    cl::CommandQueue queue;

    PerformanceCounter counter;
};


struct CorrelatorTest : public UnitTest
{
  CorrelatorTest(const Parset &ps)
  :
    //UnitTest(ps, "Correlator.cl")
    UnitTest(ps, "NewCorrelator.cl")
  {
    if (ps.nrStations() >= 5 && ps.nrChannelsPerSubband() >= 6 && ps.nrSamplesPerChannel() >= 100) {
      MultiArraySharedBuffer<std::complex<float>, 4> inputData(boost::extents[ps.nrStations()][ps.nrChannelsPerSubband()][ps.nrSamplesPerChannel()][NR_POLARIZATIONS], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY);
      MultiArraySharedBuffer<std::complex<float>, 4> visibilities(boost::extents[ps.nrBaselines()][ps.nrChannelsPerSubband()][NR_POLARIZATIONS][NR_POLARIZATIONS], queue, CL_MEM_READ_ONLY, CL_MEM_WRITE_ONLY);
      CorrelatorKernel correlator(ps, queue, program, visibilities, inputData);

      //inputData[3][5][99][1] = std::complex<float>(3, 4);
      //inputData[4][5][99][1] = std::complex<float>(5, 6);
      inputData[2][5][99][1] = std::complex<float>(3, 4);
      inputData[65][5][99][1] = std::complex<float>(5, 6);

visibilities.hostToDevice(CL_FALSE);
      inputData.hostToDevice(CL_FALSE);
      correlator.enqueue(queue, counter);
      visibilities.deviceToHost(CL_TRUE);

      //check(visibilities[13][5][1][1], std::complex<float>(39, 2));
      //check(visibilities[5463][5][1][1], std::complex<float>(39, 2));
      for (unsigned bl = 0; bl < ps.nrBaselines(); bl ++)
	if (visibilities[bl][5][1][1] != std::complex<float>(0, 0))
	  std::cout << "bl = " << bl << ", visibility = " << visibilities[bl][5][1][1] << std::endl;
    }
  }
};


struct IncoherentStokesTest : public UnitTest
{
  IncoherentStokesTest(const Parset &ps)
  :
    UnitTest(ps, "BeamFormer/IncoherentStokes.cl")
  {
    if (ps.nrStations() >= 5 && ps.nrChannelsPerSubband() >= 14 && ps.nrSamplesPerChannel() >= 108) {
      MultiArraySharedBuffer<std::complex<float>, 4> inputData(boost::extents[ps.nrStations()][ps.nrChannelsPerSubband()][ps.nrSamplesPerChannel()][NR_POLARIZATIONS], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY);
      MultiArraySharedBuffer<float, 3> stokesData(boost::extents[ps.nrIncoherentStokes()][ps.nrSamplesPerChannel() / ps.incoherentStokesTimeIntegrationFactor()][ps.nrChannelsPerSubband()], queue, CL_MEM_READ_ONLY, CL_MEM_WRITE_ONLY);
      IncoherentStokesKernel kernel(ps, queue, program, stokesData, inputData);

      inputData[4][13][107][0] = std::complex<float>(2, 3);
      inputData[4][13][107][1] = std::complex<float>(4, 5);

      inputData.hostToDevice(CL_FALSE);
      kernel.enqueue(queue, counter);
      stokesData.deviceToHost(CL_TRUE);

      const static float expected[] = { 54, -28, 46, 4 };

      for (unsigned stokes = 0; stokes < ps.nrIncoherentStokes(); stokes ++)
	check(stokesData[stokes][107 / ps.incoherentStokesTimeIntegrationFactor()][13], expected[stokes]);
    }
  }
};


struct IntToFloatTest : public UnitTest
{
  IntToFloatTest(const Parset &ps)
  :
    UnitTest(ps, "BeamFormer/IntToFloat.cl")
  {
    if (ps.nrStations() >= 3 && ps.nrSamplesPerChannel() * ps.nrChannelsPerSubband() >= 10077) {
      MultiArraySharedBuffer<char, 4> inputData(boost::extents[ps.nrStations()][ps.nrSamplesPerChannel() * ps.nrChannelsPerSubband()][NR_POLARIZATIONS][ps.nrBytesPerComplexSample()], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY);
      MultiArraySharedBuffer<std::complex<float>, 3> outputData(boost::extents[ps.nrStations()][NR_POLARIZATIONS][ps.nrSamplesPerChannel() * ps.nrChannelsPerSubband()], queue, CL_MEM_READ_ONLY, CL_MEM_WRITE_ONLY);
      IntToFloatKernel kernel(ps, queue, program, outputData, inputData);

      switch (ps.nrBytesPerComplexSample()) {
	case 4 : reinterpret_cast<std::complex<short> &>(inputData[2][10076][1][0]) = 7;
		 break;

	case 2 : reinterpret_cast<std::complex<signed char> &>(inputData[2][10076][1][0]) = 7;
		 break;

	case 1 : reinterpret_cast<i4complex &>(inputData[2][10076][1][0]) = i4complex(7, 0);
		 break;
      }

      inputData.hostToDevice(CL_FALSE);
      kernel.enqueue(queue, counter);
      outputData.deviceToHost(CL_TRUE);
      check(outputData[2][1][10076], std::complex<float>(7.0f, 0));
    }
  }
};


struct BeamFormerTest : public UnitTest
{
  BeamFormerTest(const Parset &ps)
  :
    UnitTest(ps, "BeamFormer/BeamFormer.cl")
  {
    if (ps.nrStations() >= 5 && ps.nrSamplesPerChannel() >= 13 && ps.nrChannelsPerSubband() >= 7 && ps.nrTABs(SAP) >= 6) {
      MultiArraySharedBuffer<std::complex<float>, 4> inputData(boost::extents[ps.nrStations()][ps.nrChannelsPerSubband()][ps.nrSamplesPerChannel()][NR_POLARIZATIONS], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY);
      MultiArraySharedBuffer<std::complex<float>, 3> beamFormerWeights(boost::extents[ps.nrStations()][ps.nrChannelsPerSubband()][ps.nrTABs(SAP)], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY);
      MultiArraySharedBuffer<std::complex<float>, 4> complexVoltages(boost::extents[ps.nrChannelsPerSubband()][ps.nrSamplesPerChannel()][ps.nrTABs(SAP)][NR_POLARIZATIONS], queue, CL_MEM_READ_ONLY, CL_MEM_READ_WRITE);
      BeamFormerKernel beamFormer(ps, program, complexVoltages, inputData, beamFormerWeights);

      inputData[4][6][12][1] = std::complex<float>(2.2, 3);
      beamFormerWeights[4][6][5] = std::complex<float>(4, 5);

      inputData.hostToDevice(CL_FALSE);
      beamFormerWeights.hostToDevice(CL_FALSE);
      beamFormer.enqueue(queue, counter);
      complexVoltages.deviceToHost(CL_TRUE);

      check(complexVoltages[6][12][5][1], std::complex<float>(-6.2, 23));

#if 0
      for (unsigned tab = 0; tab < ps.nrTABs(SAP); tab ++)
	for (unsigned pol = 0; pol < NR_POLARIZATIONS; pol ++)
	  for (unsigned ch = 0; ch < ps.nrChannelsPerSubband(); ch ++)
	    for (unsigned t = 0; t < ps.nrSamplesPerChannel(); t ++)
	      if (complexVoltages[tab][pol][ch][t] != std::complex<float>(0, 0))
		std::cout << "complexVoltages[" << tab << "][" << pol << "][" << ch << "][" << t << "] = " << complexVoltages[tab][pol][ch][t] << std::endl;
#endif
    }
  }
};


struct BeamFormerTransposeTest : public UnitTest
{
  BeamFormerTransposeTest(const Parset &ps)
  :
    UnitTest(ps, "BeamFormer/Transpose.cl")
  {
    if (ps.nrChannelsPerSubband() >= 19 && ps.nrSamplesPerChannel() >= 175 && ps.nrTABs(SAP) >= 5) {
      MultiArraySharedBuffer<std::complex<float>, 4> transposedData(boost::extents[ps.nrTABs(SAP)][NR_POLARIZATIONS][ps.nrSamplesPerChannel()][ps.nrChannelsPerSubband()], queue, CL_MEM_READ_ONLY, CL_MEM_WRITE_ONLY);
      MultiArraySharedBuffer<std::complex<float>, 4> complexVoltages(boost::extents[ps.nrChannelsPerSubband()][ps.nrSamplesPerChannel()][ps.nrTABs(SAP)][NR_POLARIZATIONS], queue, CL_MEM_READ_WRITE, CL_MEM_READ_ONLY);
      BeamFormerTransposeKernel transpose(ps, program, transposedData, complexVoltages);

      complexVoltages[18][174][4][1] = std::complex<float>(24, 42);

      complexVoltages.hostToDevice(CL_FALSE);
      transpose.enqueue(queue, counter);
      transposedData.deviceToHost(CL_TRUE);

      check(transposedData[4][1][174][18], std::complex<float>(24, 42));
    }
  }
};


struct DedispersionChirpTest : public UnitTest
{
  DedispersionChirpTest(const Parset &ps)
  :
    UnitTest(ps, "BeamFormer/Dedispersion.cl")
  {
    if (ps.nrTABs(SAP) > 3 && ps.nrChannelsPerSubband() > 13 && ps.nrSamplesPerChannel() / ps.dedispersionFFTsize() > 1 && ps.dedispersionFFTsize() > 77) {
      MultiArraySharedBuffer<std::complex<float>, 5> data(boost::extents[ps.nrTABs(SAP)][NR_POLARIZATIONS][ps.nrChannelsPerSubband()][ps.nrSamplesPerChannel() / ps.dedispersionFFTsize()][ps.dedispersionFFTsize()], queue, CL_MEM_READ_WRITE, CL_MEM_READ_WRITE);
      MultiArraySharedBuffer<float, 1> DMs(boost::extents[ps.nrTABs(SAP)], queue, CL_MEM_READ_ONLY, CL_MEM_WRITE_ONLY);
      DedispersionChirpKernel dedispersionChirpKernel(ps, program, queue, data, DMs);

      data[3][1][13][1][77] = std::complex<float>(2, 3);
      DMs[3] = 2;

      DMs.hostToDevice(CL_FALSE);
      data.hostToDevice(CL_FALSE);
      dedispersionChirpKernel.enqueue(queue, counter, 60e6);
      data.deviceToHost(CL_TRUE);

      std::cout << data[3][1][13][1][77] << std::endl;
    }
  }
};


struct CoherentStokesTest : public UnitTest
{
  CoherentStokesTest(const Parset &ps)
  :
    UnitTest(ps, "BeamFormer/CoherentStokes.cl")
  {
    if (ps.nrChannelsPerSubband() >= 19 && ps.nrSamplesPerChannel() >= 175 && ps.nrTABs(SAP) >= 5) {
      MultiArraySharedBuffer<float, 4> stokesData(boost::extents[ps.nrTABs(SAP)][ps.nrCoherentStokes()][ps.nrSamplesPerChannel() / ps.coherentStokesTimeIntegrationFactor()][ps.nrChannelsPerSubband()], queue, CL_MEM_READ_ONLY, CL_MEM_WRITE_ONLY);
#if 1
      MultiArraySharedBuffer<std::complex<float>, 4> complexVoltages(boost::extents[ps.nrChannelsPerSubband()][ps.nrSamplesPerChannel()][ps.nrTABs(SAP)][NR_POLARIZATIONS], queue, CL_MEM_READ_WRITE, CL_MEM_READ_ONLY);
      CoherentStokesKernel stokesKernel(ps, program, stokesData, complexVoltages);

      complexVoltages[18][174][4][0] = std::complex<float>(2, 3);
      complexVoltages[18][174][4][1] = std::complex<float>(4, 5);
#else
      MultiArraySharedBuffer<std::complex<float>, 4> complexVoltages(boost::extents[ps.nrTABs(SAP)][NR_POLARIZATIONS][ps.nrSamplesPerChannel()][ps.nrChannelsPerSubband()], queue, CL_MEM_READ_WRITE, CL_MEM_READ_ONLY);
      CoherentStokesKernel stokesKernel(ps, program, stokesData, complexVoltages);

      complexVoltages[18][174][4][0] = std::complex<float>(2, 3);
      complexVoltages[18][174][4][1] = std::complex<float>(4, 5);
#endif

      complexVoltages.hostToDevice(CL_FALSE);
      stokesKernel.enqueue(queue, counter);
      stokesData.deviceToHost(CL_TRUE);

      for (unsigned stokes = 0; stokes < ps.nrCoherentStokes(); stokes ++)
	std::cout << stokesData[4][stokes][174 / ps.coherentStokesTimeIntegrationFactor()][18] << std::endl;
    }
  }
};


struct UHEP_BeamFormerTest : public UnitTest
{
  UHEP_BeamFormerTest(const Parset &ps)
  :
    UnitTest(ps, "UHEP/BeamFormer.cl")
  {
    if (ps.nrStations() >= 5 && (ps.nrSamplesPerChannel() + NR_STATION_FILTER_TAPS - 1) >= 13 && ps.nrSubbands() >= 7 && ps.nrTABs(SAP) >= 6) {
      MultiArraySharedBuffer<char, 5> inputSamples(boost::extents[ps.nrStations()][ps.nrSubbands()][ps.nrSamplesPerChannel() + NR_STATION_FILTER_TAPS - 1][NR_POLARIZATIONS][ps.nrBytesPerComplexSample()], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY);
      MultiArraySharedBuffer<std::complex<float>, 3> beamFormerWeights(boost::extents[ps.nrStations()][ps.nrSubbands()][ps.nrTABs(SAP)], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY);
      MultiArraySharedBuffer<std::complex<float>, 4> complexVoltages(boost::extents[ps.nrSubbands()][ps.nrSamplesPerChannel() + NR_STATION_FILTER_TAPS - 1][ps.nrTABs(SAP)][NR_POLARIZATIONS], queue, CL_MEM_READ_ONLY, CL_MEM_READ_WRITE);
      UHEP_BeamFormerKernel beamFormer(ps, program, complexVoltages, inputSamples, beamFormerWeights);

      switch (ps.nrBytesPerComplexSample()) {
	case 4 : reinterpret_cast<std::complex<short> &>(inputSamples[4][6][12][1][0]) = std::complex<short>(2, 3);
		 break;

	case 2 : reinterpret_cast<std::complex<signed char> &>(inputSamples[4][6][12][1][0]) = std::complex<signed char>(2, 3);
		 break;

	case 1 : reinterpret_cast<i4complex &>(inputSamples[4][6][12][1][0]) = i4complex(2, 3);
		 break;
      }

      beamFormerWeights[4][6][5] = std::complex<float>(4, 5);

      inputSamples.hostToDevice(CL_FALSE);
      beamFormerWeights.hostToDevice(CL_FALSE);
      beamFormer.enqueue(queue, counter);
      complexVoltages.deviceToHost(CL_TRUE);

      check(complexVoltages[6][12][5][1], std::complex<float>(-7, 22));
    }
  }
};


struct UHEP_TransposeTest : public UnitTest
{
  UHEP_TransposeTest(const Parset &ps)
  :
    UnitTest(ps, "UHEP/Transpose.cl")
  {
    if (ps.nrSubbands() >= 19 && ps.nrSamplesPerChannel() + NR_STATION_FILTER_TAPS - 1 >= 175 && ps.nrTABs(SAP) >= 5) {
      MultiArraySharedBuffer<std::complex<float>, 4> transposedData(boost::extents[ps.nrTABs(SAP)][NR_POLARIZATIONS][ps.nrSamplesPerChannel() + NR_STATION_FILTER_TAPS - 1][512], queue, CL_MEM_READ_ONLY, CL_MEM_WRITE_ONLY);
      MultiArraySharedBuffer<std::complex<float>, 4> complexVoltages(boost::extents[ps.nrSubbands()][ps.nrSamplesPerChannel() + NR_STATION_FILTER_TAPS - 1][ps.nrTABs(SAP)][NR_POLARIZATIONS], queue, CL_MEM_READ_WRITE, CL_MEM_READ_ONLY);
      cl::Buffer devReverseSubbandMapping(context, CL_MEM_READ_ONLY, 512 * sizeof(int));
      UHEP_TransposeKernel transpose(ps, program, transposedData, complexVoltages, devReverseSubbandMapping);

      complexVoltages[18][174][4][1] = std::complex<float>(24, 42);

      queue.enqueueWriteBuffer(devReverseSubbandMapping, CL_FALSE, 0, 512 * sizeof(int), reverseSubbandMapping);
      complexVoltages.hostToDevice(CL_FALSE);
      transpose.enqueue(queue, counter);
      transposedData.deviceToHost(CL_TRUE);

      check(transposedData[4][1][174][38], std::complex<float>(24, 42));
    }
  }
};


struct UHEP_TriggerTest : public UnitTest
{
  UHEP_TriggerTest(const Parset &ps)
  :
    UnitTest(ps, "UHEP/Trigger.cl")
  {
    if (ps.nrTABs(SAP) >= 4 && 1024 * ps.nrSamplesPerChannel() > 100015) {
      MultiArraySharedBuffer<float, 3> inputData(boost::extents[ps.nrTABs(SAP)][NR_POLARIZATIONS][ps.nrSamplesPerChannel() * 1024], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY);
      MultiArraySharedBuffer<TriggerInfo, 1> triggerInfo(boost::extents[ps.nrTABs(SAP)], queue, CL_MEM_READ_ONLY, CL_MEM_WRITE_ONLY);
      UHEP_TriggerKernel trigger(ps, program, triggerInfo, inputData);

      inputData[3][1][100015] = 1000;

      inputData.hostToDevice(CL_FALSE);
      trigger.enqueue(queue, counter);
      triggerInfo.deviceToHost(CL_TRUE);

      std::cout << "trigger info: mean = " << triggerInfo[3].mean << ", variance = " << triggerInfo[3].variance << ", bestValue = " << triggerInfo[3].bestValue << ", bestApproxIndex = " << triggerInfo[3].bestApproxIndex << std::endl;
      //check(triggerInfo[3].mean, (float) (1000.0f * 1000.0f) / (float) (ps.nrSamplesPerChannel() * 1024));
      check(triggerInfo[3].bestValue, 1000.0f * 1000.0f);
      check(triggerInfo[3].bestApproxIndex, 100016U);
    }
  }
};


#if 0
struct FFT_Test : public UnitTest
{
  FFT_Test(const Parset &ps)
  : UnitTest(ps, "fft.cl")
  {
    MultiArraySharedBuffer<std::complex<float>, 1> in(boost::extents[8], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY);
    MultiArraySharedBuffer<std::complex<float>, 1> out(boost::extents[8], queue, CL_MEM_READ_ONLY, CL_MEM_WRITE_ONLY);

    for (unsigned i = 0; i < 8; i ++)
      in[i] = std::complex<float>(2 * i + 1, 2 * i + 2);

    clAmdFftSetupData setupData;
    cl::detail::errHandler(clAmdFftInitSetupData(&setupData), "clAmdFftInitSetupData");
    setupData.debugFlags = CLFFT_DUMP_PROGRAMS;
    cl::detail::errHandler(clAmdFftSetup(&setupData), "clAmdFftSetup");

    clAmdFftPlanHandle plan;
    size_t dim[1] = { 8 };

    cl::detail::errHandler(clAmdFftCreateDefaultPlan(&plan, context(), CLFFT_1D, dim), "clAmdFftCreateDefaultPlan");
    cl::detail::errHandler(clAmdFftSetResultLocation(plan, CLFFT_OUTOFPLACE), "clAmdFftSetResultLocation");
    cl::detail::errHandler(clAmdFftSetPlanBatchSize(plan, 1), "clAmdFftSetPlanBatchSize");
    cl::detail::errHandler(clAmdFftBakePlan(plan, 1, &queue(), 0, 0), "clAmdFftBakePlan");

    in.hostToDevice(CL_FALSE);
    cl_mem ins[1] = { ((cl::Buffer) in)() };
    cl_mem outs[1] = { ((cl::Buffer) out)() };
#if 1
    cl::detail::errHandler(clAmdFftEnqueueTransform(plan, CLFFT_FORWARD, 1, &queue(), 0, 0, 0, ins, outs, 0), "clAmdFftEnqueueTransform");
#else
    cl::Kernel kernel(program, "fft_fwd");
    kernel.setArg(0, (cl::Buffer) in);
    kernel.setArg(1, (cl::Buffer) out);
    queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(64, 1, 1), cl::NDRange(64, 1, 1));
#endif
    out.deviceToHost(CL_TRUE);

    for (unsigned i = 0; i < 8; i ++)
      std::cout << out[i] << std::endl;

    cl::detail::errHandler(clAmdFftDestroyPlan(&plan), "clAmdFftDestroyPlan");
    cl::detail::errHandler(clAmdFftTeardown(), "clAmdFftTeardown");
  }
};
#endif


} // namespace RTCP
} // namespace LOFAR

int main(int argc, char **argv)
{
  using namespace LOFAR::RTCP;

  std::cout << "running ..." << std::endl;

  if (setenv("DISPLAY", ":0.0", 1) < 0) {
    perror("error setting DISPLAY");
    exit(1);
  }

  if (argc != 2) {
    std::cerr << "usage: " << argv[0] << " parset" << std::endl;
    exit(1);
  }

#if 0 && defined __linux__
  set_affinity(0);
#endif

  try {
    Parset ps(argv[1]);

#if 0
    const char *str = getenv("NR_STATIONS");
    ps.nrStations() = str ? atoi(str) : 77;
#endif
    std::cout << "nr stations = " << ps.nrStations() << std::endl;

    const char *str = getenv("NR_GPUS");
    nrGPUs = str ? atoi(str) : 1;

#if 0
    ps.nrSubbands()		= 10;//488;
    ps.nrChannelsPerSubband()	= 64;
    ps.nrBeams()		= 1;
    ps.nrSamplesPerChannel()	= 196608 / ps.nrChannelsPerSubband();
    ps.subbandBandwidth()	= 195312.5;
    ps.correctBandPass()	= true;
#endif

    //profiling = false; CorrelatorPipeline(ps).doWork();
    //profiling = true; CorrelatorPipeline(ps).doWork();

    (CorrelatorTest)(ps);

#if 0
    ps.nrSubbands()		  = 488;
    ps.nrChannelsPerSubband()	  = 2048;
    ps.nrBeams()		  = 1;
    ps.nrTABs(SAP)			  = 128;
    ps.nrIncoherentStokes()	  = 4;
    ps.nrCoherentStokes()	  = 4;
    ps.incoherentStokesTimeIntegrationFactor() = 8;
    ps.coherentStokesTimeIntegrationFactor() = 8;
    ps.nrSamplesPerChannel()	  = 65536 / ps.nrChannelsPerSubband();//262144 / ps.nrChannelsPerSubband();
    ps.subbandBandwidth()	  = 195312.5;
    ps.correctBandPass()	  = true;
    ps.dedispersionFFTsize()	  = ps.nrSamplesPerChannel();
  
    profiling = false; BeamFormerPipeline(ps).doWork();
    profiling = true; BeamFormerPipeline(ps).doWork();
    //(IncoherentStokesTest)(ps);
    //(IntToFloatTest)(ps);
    //(BeamFormerTest)(ps);
    //(BeamFormerTransposeTest)(ps);
    //(DedispersionChirpTest)(ps);
    //(CoherentStokesTest)(ps);
#endif

#if 0
    ps.nrSubbands()	   = 488;
    ps.nrSamplesPerChannel() = 1024;
    ps.nrBeams()	   = 1;
    ps.subbandBandwidth()  = 195312.5;
    ps.nrTABs(SAP)		   = 48;

    profiling = false; UHEP_Pipeline(ps).doWork();
    profiling = true; UHEP_Pipeline(ps).doWork();
    //(UHEP_BeamFormerTest)(ps);
    //(UHEP_TransposeTest)(ps);
    //(UHEP_TriggerTest)(ps);
#endif

#if 0
    (FFT_Test)(ps);
#endif
  } catch (cl::Error &error) {
#pragma omp critical (cerr)
    std::cerr << "OpenCL error: " << error.what() << ": " << errorMessage(error.err()) << std::endl << error;
    exit(1);
  }

  return 0;
}