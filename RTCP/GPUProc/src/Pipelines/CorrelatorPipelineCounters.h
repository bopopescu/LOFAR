#ifndef GPUPROC_CORRELATORPIPELINECOUNTERS_H
#define GPUPROC_CORRELATORPIPELINECOUNTERS_H
#include "global_defines.h"

#include "PerformanceCounter.h"
#include <string>

namespace LOFAR
{
  namespace RTCP 
  {
    struct CorrelatorPipelineCounters
    {
      CorrelatorPipelineCounters(
#if defined USE_NEW_CORRELATOR
      const std::string &correlateTriangleCounterName,
      const std::string &correlateRectangleCounterName,
#else
      const std::string &correlatorCounterName,
#endif
      const std::string &firFilterCounterName, 
      const std::string &delayAndBandPassCounterName,
      const std::string &fftCounterName,
      const std::string &samplesCounterName,
      const std::string &visibilitiesCounterName)
      :
#if defined USE_NEW_CORRELATOR
      correlateTriangleCounter(correlateTriangleCounterName),
      correlateRectangleCounter(correlateRectangleCounterName),
#else
      correlatorCounter(correlatorCounterName),
#endif
      firFilterCounter(firFilterCounterName),
      delayAndBandPassCounter(delayAndBandPassCounterName),
      fftCounter(fftCounterName),
      samplesCounter(samplesCounterName),
      visibilitiesCounter(visibilitiesCounterName)
      {};

#if defined USE_NEW_CORRELATOR
      PerformanceCounter correlateTriangleCounter;
      PerformanceCounter correlateRectangleCounter;
#else
      PerformanceCounter correlatorCounter;
#endif
      PerformanceCounter firFilterCounter;
      PerformanceCounter delayAndBandPassCounter;
      PerformanceCounter fftCounter;
      PerformanceCounter samplesCounter;
      PerformanceCounter visibilitiesCounter;

    };
  }
}
#endif