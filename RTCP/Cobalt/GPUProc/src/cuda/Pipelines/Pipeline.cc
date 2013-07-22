//# Pipeline.cc
//# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#include <lofar_config.h>

#include "Pipeline.h"

#include <Common/LofarLogger.h>
#include <Common/lofar_iomanip.h>
#include <ApplCommon/PosixTime.h>
#include <Stream/Stream.h>
#include <Stream/FileStream.h>
#include <Stream/NullStream.h>

#include <CoInterface/Stream.h>
#include <GPUProc/gpu_utils.h>
#include <GPUProc/Kernels/Kernel.h>
#include <GPUProc/SubbandProcs/SubbandProc.h>
#include <InputProc/SampleType.h>

#ifdef HAVE_MPI
#include <InputProc/Transpose/MPIReceiveStations.h>
#else
#include <GPUProc/Station/StationInput.h>
#endif

namespace LOFAR
{
  namespace Cobalt
  {


    Pipeline::Pipeline(const Parset &ps, const std::vector<size_t> &subbandIndices)
      :
      ps(ps),
      platform(),
      devices(platform.devices()),
      subbandIndices(subbandIndices),
      performance(devices.size()),
      subbandPool(subbandIndices.size())
    {
    }


    // Record type needed by receiveInput. Before c++0x, a local type
    // can't be a template argument, so we'll have to define this type
    // globally.
    struct inputData_t {
      // An InputData object suited for storing one subband from all
      // stations.
      SmartPtr<SubbandProcInputData> data;

      // The SubbandProc associated with the data
      SubbandProc *queue;
    };

    template<typename SampleT> void Pipeline::receiveInput( size_t nrBlocks )
    {
      // Need SubbandProcs to send work to
      ASSERT(workQueues.size() > 0);

      size_t workQueueIterator = 0;

      // The length of a block in samples
      size_t blockSize = ps.nrHistorySamples() + ps.nrSamplesPerSubband();

      // RECEIVE: Set up to receive our subbands as indicated by subbandIndices
#ifdef HAVE_MPI
      MPIReceiveStations receiver(ps.nrStations(), subbandIndices, blockSize);
#else
      DirectInput &receiver = DirectInput::instance();
#endif

      // Create a block object to hold all information for receiving one
      // block.
      vector<struct ReceiveStations::Block<SampleT> > blocks(ps.nrStations());

      for (size_t stat = 0; stat < ps.nrStations(); ++stat) {
        blocks[stat].beamlets.resize(subbandIndices.size());
      }

      for (size_t block = 0; block < nrBlocks; block++) {
        // Receive the samples of all subbands from the stations for this
        // block.

        LOG_INFO_STR("[block " << block << "] Reading input samples");

        // The set of InputData objects we're using for this block.
        vector<struct inputData_t> inputDatas(subbandIndices.size());

        for (size_t inputIdx = 0; inputIdx < inputDatas.size(); ++inputIdx) {
          // Fetch an input object to store this inputIdx. For now, blindly
          // round-robin over the work queues.
          SubbandProc &queue = *workQueues[workQueueIterator++ % workQueues.size()];

          // Fetch an input object to fill from the selected queue.
          // NOTE: We'll put it in a SmartPtr right away!
          SmartPtr<SubbandProcInputData> data = queue.inputPool.free.remove();

          // Annotate the block
          struct BlockID id;
          id.block            = block;
          id.globalSubbandIdx = subbandIndices[inputIdx];
          id.localSubbandIdx  = inputIdx;
          data->blockID = id;

          for (size_t stat = 0; stat < ps.nrStations(); ++stat) {
            // Incorporate it in the receiver's input set.
            blocks[stat].beamlets[inputIdx].samples = reinterpret_cast<SampleT*>(&data->inputSamples[stat][0][0][0]);
          }

          // Record the block (transfers ownership)
          inputDatas[inputIdx].data = data;
          inputDatas[inputIdx].queue = &queue;
        }

        // Receive all subbands from all stations
        receiver.receiveBlock<SampleT>(blocks);

        size_t nrFlaggedSamples = 0;

        // Process and forward the received input to the processing threads
        for (size_t inputIdx = 0; inputIdx < inputDatas.size(); ++inputIdx) {
          SubbandProc &queue = *inputDatas[inputIdx].queue;
          SmartPtr<SubbandProcInputData> data = inputDatas[inputIdx].data;

          const unsigned SAP = ps.settings.subbands[data->blockID.globalSubbandIdx].SAP;

          // Translate the metadata as provided by receiver
          for (size_t stat = 0; stat < ps.nrStations(); ++stat) {
            SubbandMetaData &metaData = blocks[stat].beamlets[inputIdx].metaData;

            // TODO: Not in this thread! Add a preprocess thread maybe?
            data->applyMetaData(ps, stat, SAP, metaData);

            nrFlaggedSamples += metaData.flags.count();
          }

          queue.inputPool.filled.append(data);
        }

        LOG_INFO_STR("[block " << block << "] Flags: " << (100 * nrFlaggedSamples / inputDatas.size() / blockSize) << "%");

        LOG_DEBUG_STR("[block " << block << "] Forwarded input to processing");
      }

      // Signal end of input
      for (size_t i = 0; i < workQueues.size(); ++i) {
        workQueues[i]->inputPool.filled.append(NULL);
      }
    }

    template void Pipeline::receiveInput< SampleType<i16complex> >( size_t nrBlocks );
    template void Pipeline::receiveInput< SampleType<i8complex> >( size_t nrBlocks );
    template void Pipeline::receiveInput< SampleType<i4complex> >( size_t nrBlocks );

    void Pipeline::receiveInput( size_t nrBlocks )
    {
      switch (ps.nrBitsPerSample()) {
      default:
      case 16:
        receiveInput< SampleType<i16complex> >(nrBlocks);
        break;
      case 8:
        receiveInput< SampleType<i8complex> >(nrBlocks);
        break;
      case 4:
        receiveInput< SampleType<i4complex> >(nrBlocks);
        break;
      }
    }


    void Pipeline::processObservation(OutputType outputType)
    {
      for (size_t i = 0; i < subbandPool.size(); i++) {
        // Allow 10 blocks to be in the best-effort queue.
        // TODO: make this dynamic based on memory or time
        subbandPool[i].bequeue = new BestEffortQueue< SmartPtr<StreamableData> >(3, ps.realTime());
      }

      double startTime = ps.startTime();
      double stopTime = ps.stopTime();
      double blockTime = ps.CNintegrationTime();

      size_t nrBlocks = floor((stopTime - startTime) / blockTime);

      //sections = program segments defined by the following omp section directive
      //           are distributed for parallel execution among available threads
      //parallel = directive explicitly instructs the compiler to parallelize the chosen block of code.
      //  The two sections in this function are done in parallel with a seperate set of threads.
#     pragma omp parallel sections
      {
        /*
         * BLOCK OF SUBBANDS -> WORKQUEUE INPUTPOOL
         */
#       pragma omp section
        {
          receiveInput(nrBlocks);
        }


        /*
         * WORKQUEUE INPUTPOOL -> WORKQUEUE OUTPUTPOOL
         *
         * Perform GPU processing, one thread per workQueue.
         */
#       pragma omp section
        {
#         pragma omp parallel for num_threads(workQueues.size())
          for (size_t i = 0; i < workQueues.size(); ++i) {
#ifdef USE_B7015
            unsigned gpuNr = i % devices.size();
            set_affinity(gpuNr);
#endif
            SubbandProc &queue = *workQueues[i];

            // run the queue
            queue.timers["CPU - total"]->start();
            processSubbands(queue);
            queue.timers["CPU - total"]->stop();

            // Signal end of output
            queue.outputPool.filled.append(NULL);
          }
        }

        /*
         * WORKQUEUE OUTPUTPOOL -> SUBBANDPOOL
         *
         * Perform post-processing, one thread per workQueue.
         */
#       pragma omp section
        {
#         pragma omp parallel for num_threads(workQueues.size())
          for (size_t i = 0; i < workQueues.size(); ++i) {
            SubbandProc &queue = *workQueues[i];

            // run the queue
            postprocessSubbands(queue);
          }

          // Signal end of output
          for (size_t i = 0; i < subbandPool.size(); ++i) {
            subbandPool[i].bequeue->noMore();
          }
        }

        /*
         * SUBBANDPOOL -> STORAGE STREAMS (best effort)
         */
#       pragma omp section
        {
#         pragma omp parallel for num_threads(subbandPool.size())
          for (size_t i = 0; i < subbandPool.size(); ++i) {
            SmartPtr<Stream> outputStream = connectToOutput(subbandIndices[i], outputType);

            // write subband to Storage
            writeSubband(subbandIndices[i], subbandPool[i], outputStream);
          }
        }
      }

      // gather performance figures
      for (size_t i = 0; i < workQueues.size(); ++i ) {
        performance.addQueue(*workQueues[i]);
      }

      // log performance figures
      performance.log(workQueues.size());
    }


    void Pipeline::processSubbands(SubbandProc &workQueue)
    {
      SmartPtr<SubbandProcInputData> input;

      // Keep fetching input objects until end-of-input
      while ((input = workQueue.inputPool.filled.remove()) != NULL) {
        const struct BlockID id = input->blockID;

        LOG_INFO_STR("[" << id << "] Processing start");

        // Also fetch an output object to store results
        SmartPtr<StreamableData> output = workQueue.outputPool.free.remove();
        ASSERT(output != NULL); // Only we signal end-of-data, so we should never receive it

        output->blockID = id;

        // Perform calculations
        workQueue.timers["CPU - process"]->start();
        workQueue.processSubband(*input, *output);
        workQueue.timers["CPU - process"]->stop();

        // Hand off output to post processing
        workQueue.outputPool.filled.append(output);
        ASSERT(!output);

        // Give back input data for a refill
        workQueue.inputPool.free.append(input);
        ASSERT(!input);

        LOG_DEBUG_STR("[" << id << "] Forwarded output to post processing");
      }
    }


    void Pipeline::SubbandProcOwnerMap::push(const struct BlockID &id, SubbandProc &workQueue)
    {
      ScopedLock sl(mutex);

      ASSERT(ownerMap.find(id) == ownerMap.end());
      ownerMap[id] = &workQueue;
    }


    SubbandProc& Pipeline::SubbandProcOwnerMap::pop(const struct BlockID &id)
    {
      SubbandProc *workQueue;

      ScopedLock sl(mutex);

      ASSERT(ownerMap.find(id) != ownerMap.end());
      workQueue = ownerMap[id];
      ASSERT(workQueue != NULL);

      ownerMap.erase(id);

      return *workQueue;
    }


    void Pipeline::postprocessSubbands(SubbandProc &workQueue)
    {
      SmartPtr<StreamableData> output;

      size_t nrBlocksForwarded = 0;
      size_t nrBlocksDropped = 0;
      time_t lastLogTime = 0;

      // Keep fetching output objects until end-of-output
      while ((output = workQueue.outputPool.filled.remove()) != NULL) {
        const struct BlockID id = output->blockID;

        LOG_INFO_STR("[" << id << "] Post processing start");

        workQueue.timers["CPU - postprocess"]->start();
        workQueue.postprocessSubband(*output);
        workQueue.timers["CPU - postprocess"]->stop();

        // Hand off output, force in-order as Storage expects it that way
        struct Output &pool = subbandPool[id.localSubbandIdx];

        pool.sync.waitFor(id.block);

        // Register ownership
        workQueueOwnerMap.push(id, workQueue);

        // We do the ordering, so we set the sequence numbers
        output->setSequenceNumber(id.block);

        if (!pool.bequeue->append(output)) {
          nrBlocksDropped++;
          //LOG_WARN_STR("[block " << block << "] Dropped for subband " << globalSubbandIdx);

          // Give back to queue
          workQueue.outputPool.free.append(output);
        } else {
          nrBlocksForwarded++;
        }

        // Allow next block to be written
        pool.sync.advanceTo(id.block + 1);

        ASSERT(!output);

        LOG_DEBUG_STR("[" << id << "] Forwarded output to writer");

        if (time(0) != lastLogTime) {
          lastLogTime = time(0);

          LOG_INFO_STR("Forwarded " << nrBlocksForwarded << " blocks, dropped " << nrBlocksDropped << " blocks");
        }
      }
    }


    void Pipeline::writeSubband( unsigned globalSubbandIdx, struct Output &output, SmartPtr<Stream> outputStream )
    {
      SmartPtr<StreamableData> outputData;

      // Process pool elements until end-of-output
      while ((outputData = output.bequeue->remove()) != NULL) {
        const struct BlockID id = outputData->blockID;
        ASSERT( globalSubbandIdx == id.globalSubbandIdx );

        // Cache workQueue reference, because `output' will be destroyed.
        SubbandProc &workQueue = workQueueOwnerMap.pop(id);

        LOG_INFO_STR("[" << id << "] Writing start");

        // Write block to disk 
        try {
          outputData->write(outputStream.get(), true);
        } catch (Exception &ex) {
          LOG_ERROR_STR("Dropping rest of subband " << id.globalSubbandIdx << ": " << ex);

          outputStream = new NullStream;
        }

        // Hand the object back to the workQueue it originally came from
        workQueue.outputPool.free.append(outputData);

        ASSERT(!outputData);

        LOG_INFO_STR("[" << id << "] Done");
      }
    }


    SmartPtr<Stream> Pipeline::connectToOutput(unsigned globalSubbandIdx, OutputType outputType) const
    {
      SmartPtr<Stream> outputStream;

      try {
        if (ps.getHostName(outputType, globalSubbandIdx) == "") {
          // an empty host name means 'write to disk directly', to
          // make debugging easier for now
          outputStream = new FileStream(ps.getFileName(outputType, globalSubbandIdx), 0666);
        } else {
          // connect to the output process for this output
          const std::string desc = getStreamDescriptorBetweenIONandStorage(ps, outputType, globalSubbandIdx);
          outputStream = createStream(desc, false, 0);
        }
      } catch (Exception &ex) {
        LOG_ERROR_STR("Failed to connect to output proc; dropping rest of subband " << globalSubbandIdx << ": " << ex);

        outputStream = new NullStream;
      }

      return outputStream;
    }


    Pipeline::Performance::Performance(size_t nrGPUs):
      nrGPUs(nrGPUs)
    {
    }


    void Pipeline::Performance::addQueue(SubbandProc &queue)
    {
      ScopedLock sl(totalsMutex);

      // add performance counters
      for (map<string, SmartPtr<PerformanceCounter> >::iterator i = queue.counters.begin(); i != queue.counters.end(); ++i) {

        const string &name = i->first;
        PerformanceCounter *counter = i->second.get();

        counter->waitForAllOperations();

        total_counters[name] += counter->getTotal();
      }

      // add timers
      for (map<string, SmartPtr<NSTimer> >::iterator i = queue.timers.begin(); i != queue.timers.end(); ++i) {

        const string &name = i->first;
        NSTimer *timer = i->second.get();

        if (!total_timers[name])
          total_timers[name] = new NSTimer(name, false, false);

        *total_timers[name] += *timer;
      }
    }
    
    void Pipeline::Performance::log(size_t nrSubbandProcs)
    {
      // Group figures based on their prefix before " - ", so "compute - FIR"
      // belongs to group "compute".
      map<string, PerformanceCounter::figures> counter_groups;

      for (map<string, PerformanceCounter::figures>::const_iterator i = total_counters.begin(); i != total_counters.end(); ++i) {
        size_t n = i->first.find(" - ");

        // discard counters without group
        if (n == string::npos)
          continue;

        // determine group name
        string group = i->first.substr(0, n);

        // add to group
        counter_groups[group] += i->second;
      }

      // Log all performance totals at DEBUG level
      for (map<string, PerformanceCounter::figures>::const_iterator i = total_counters.begin(); i != total_counters.end(); ++i) {
        LOG_DEBUG_STR(i->second.log(i->first));
      }

      for (map<string, SmartPtr<NSTimer> >::const_iterator i = total_timers.begin(); i != total_timers.end(); ++i) {
        LOG_DEBUG_STR(*(i->second));
      }

      // Log all group totals at INFO level
      for (map<string, PerformanceCounter::figures>::const_iterator i = counter_groups.begin(); i != counter_groups.end(); ++i) {
        LOG_INFO_STR(i->second.log(i->first));
      }

      // Log specific performance figures for regression tests at INFO level
      double wall_seconds = total_timers["CPU - total"]->getAverage();
      double gpu_seconds = counter_groups["compute"].runtime / nrGPUs;
      double spin_seconds = total_timers["GPU - wait"]->getAverage();
      double input_seconds = total_timers["CPU - read input"]->getElapsed() / nrSubbandProcs;
      double cpu_seconds = total_timers["CPU - process"]->getElapsed() / nrSubbandProcs;
      double postprocess_seconds = total_timers["CPU - postprocess"]->getElapsed() / nrSubbandProcs;

      LOG_INFO_STR("Wall seconds spent processing        : " << fixed << setw(8) << setprecision(3) << wall_seconds);
      LOG_INFO_STR("GPU  seconds spent computing, per GPU: " << fixed << setw(8) << setprecision(3) << gpu_seconds);
      LOG_INFO_STR("Spin seconds spent polling, per block: " << fixed << setw(8) << setprecision(3) << spin_seconds);
      LOG_INFO_STR("CPU  seconds spent on input,   per WQ: " << fixed << setw(8) << setprecision(3) << input_seconds);
      LOG_INFO_STR("CPU  seconds spent processing, per WQ: " << fixed << setw(8) << setprecision(3) << cpu_seconds);
      LOG_INFO_STR("CPU  seconds spent postprocessing, per WQ: " << fixed << setw(8) << setprecision(3) << postprocess_seconds);
    }

  }
}

