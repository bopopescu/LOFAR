#include "lofar_config.h"    

#include "Common/LofarLogger.h"
#include "global_defines.h"
#include "CoInterface/Parset.h"
#include "OpenCL_Support.h"
#include "OpenMP_Support.h"
#include <algorithm>
#include <iostream>
#include "ApplCommon/PosixTime.h"
#include <boost/date_time/posix_time/posix_time.hpp>

#include "WorkQueue.h"

#include "Kernels/UHEP_TransposeKernel.h"
#include "Kernels/UHEP_InvFFT_Kernel.h"
#include "Kernels/UHEP_InvFIR_Kernel.h"
#include "Kernels/UHEP_TriggerKernel.h"
#include "Kernels/UHEP_BeamFormerKernel.h"

#include "UHEP/InvertedStationPPFWeights.h"

#include "UHEP_WorkQueue.h"

namespace LOFAR
{
    namespace  RTCP 
    {     
                UHEP_WorkQueue::UHEP_WorkQueue(UHEP_Pipeline &pipeline, unsigned gpuNumber)
            :
        WorkQueue( pipeline.context, pipeline.devices[gpuNumber], gpuNumber, pipeline.ps),
            pipeline(pipeline),
            hostInputSamples(boost::extents[ps.nrStations()][ps.nrSubbands()][ps.nrSamplesPerChannel() + NR_STATION_FILTER_TAPS - 1][NR_POLARIZATIONS][ps.nrBytesPerComplexSample()], queue, CL_MEM_WRITE_ONLY),
            hostBeamFormerWeights(boost::extents[ps.nrStations()][ps.nrSubbands()][ps.nrTABs(0)], queue, CL_MEM_WRITE_ONLY),
            hostTriggerInfo(ps.nrTABs(0), queue, CL_MEM_READ_ONLY)
        {
            size_t inputSamplesSize = ps.nrStations() * ps.nrSubbands() * (ps.nrSamplesPerChannel() + NR_STATION_FILTER_TAPS - 1) * NR_POLARIZATIONS * ps.nrBytesPerComplexSample();
            size_t complexVoltagesSize = ps.nrSubbands() * (ps.nrSamplesPerChannel() + NR_STATION_FILTER_TAPS - 1) * ps.nrTABs(0) * NR_POLARIZATIONS * sizeof(std::complex<float>);
            size_t transposedDataSize = ps.nrTABs(0) * NR_POLARIZATIONS * (ps.nrSamplesPerChannel() + NR_STATION_FILTER_TAPS - 1) * 512 * sizeof(std::complex<float>);
            size_t invFIRfilteredDataSize = ps.nrTABs(0) * NR_POLARIZATIONS * ps.nrSamplesPerChannel() * 512 * sizeof(std::complex<float>);

            size_t buffer0size = std::max(inputSamplesSize, transposedDataSize);
            size_t buffer1size = std::max(complexVoltagesSize, invFIRfilteredDataSize);

            devBuffers[0] = cl::Buffer(pipeline.context, CL_MEM_READ_WRITE, buffer0size);
            devBuffers[1] = cl::Buffer(pipeline.context, CL_MEM_READ_WRITE, buffer1size);

            size_t beamFormerWeightsSize = ps.nrStations() * ps.nrSubbands() * ps.nrTABs(0) * sizeof(std::complex<float>);
            devBeamFormerWeights = cl::Buffer(pipeline.context, CL_MEM_READ_ONLY, beamFormerWeightsSize);

            devInputSamples = devBuffers[0];
            devComplexVoltages = devBuffers[1];

            devReverseSubbandMapping = cl::Buffer(pipeline.context, CL_MEM_READ_ONLY, 512 * sizeof(int));
            devInvFIRfilterWeights = cl::Buffer(pipeline.context, CL_MEM_READ_ONLY, 1024 * NR_STATION_FILTER_TAPS * sizeof(float));
            devFFTedData = devBuffers[0];
            devInvFIRfilteredData = devBuffers[1];

            devTriggerInfo = cl::Buffer(pipeline.context, CL_MEM_WRITE_ONLY, ps.nrTABs(0) * sizeof(TriggerInfo));
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

            double executionStartTime = omp_get_wtime();

#pragma omp for schedule(dynamic), nowait
            for (unsigned block = 0; block < nrBlocks; block ++) {
                    double currentTime = startTime + block * blockTime;

                    //#pragma omp single nowait // FIXME: why does the compiler complain here???
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
            }

#pragma omp barrier

#pragma omp master
            if (!profiling)
#pragma omp critical (cout)
                std::cout << "run time = " << omp_get_wtime() - executionStartTime << std::endl;
        }



    }
}
