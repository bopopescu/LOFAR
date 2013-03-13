#ifndef GPUPROC_DEDISPERSIONFORWARDFFTKERNEL_H
#define GPUPROC_DEDISPERSIONFORWARDFFTKERNEL_H

#include "CL/cl.hpp"
#include "CoInterface/Parset.h"
#include "OpenCL_Support.h"
#include "FFT_Kernel.h"

namespace LOFAR
{
  namespace Cobalt
  {
    class DedispersionForwardFFTkernel : public FFT_Kernel
    {
    public:
      DedispersionForwardFFTkernel(const Parset &ps, cl::Context &context, cl::Buffer &buffer);

    };
  }
}
#endif