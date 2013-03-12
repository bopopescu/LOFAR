#ifndef LOFAR_CNPROC_INPUT_DATA_H
#define LOFAR_CNPROC_INPUT_DATA_H

#include <Common/lofar_complex.h>
#include <Common/DataConvert.h>
#include <CoInterface/Align.h>
#include <CoInterface/Allocator.h>
#include <CoInterface/MultiDimArray.h>
#include <CoInterface/Config.h>
#include <CoInterface/StreamableData.h>
#include <Stream/Stream.h>

#include <CoInterface/Allocator.h>

#include <vector>


namespace LOFAR
{
  namespace Cobalt
  {

    template <typename SAMPLE_TYPE>
    class InputData : public SampleData<SAMPLE_TYPE,3,1>
    {
    public:
      typedef SampleData<SAMPLE_TYPE,3,1> SuperType;

      InputData(unsigned nrSubbands, unsigned nrSamplesToCNProc, Allocator &allocator = heapAllocator);

      // used for asynchronous transpose
      void readOne(Stream *str, unsigned subbandPosition);

    protected:
      virtual void checkEndianness();
    };


    template <typename SAMPLE_TYPE>
    inline InputData<SAMPLE_TYPE>::InputData(unsigned nrSubbands, unsigned nrSamplesToCNProc, Allocator &allocator)
      :
      SuperType(boost::extents[nrSubbands][nrSamplesToCNProc][NR_POLARIZATIONS], boost::extents[0], allocator)
    {
    }

    // used for asynchronous transpose
    template <typename SAMPLE_TYPE>
    inline void InputData<SAMPLE_TYPE>::readOne(Stream *str, unsigned subbandPosition)
    {
      str->read(SuperType::samples[subbandPosition].origin(), SuperType::samples[subbandPosition].num_elements() * sizeof(SAMPLE_TYPE));

#if defined C_IMPLEMENTATION && defined WORDS_BIGENDIAN
      dataConvert(LittleEndian, SuperType::samples[subbandPosition].origin(), SuperType::samples[subbandPosition].num_elements());
#endif
    }

    template <typename SAMPLE_TYPE>
    inline void InputData<SAMPLE_TYPE>::checkEndianness()
    {
#if defined C_IMPLEMENTATION && defined WORDS_BIGENDIAN
      dataConvert(LittleEndian, SuperType::samples.origin(), SuperType::samples.num_elements());
#endif
    }

  } // namespace Cobalt
} // namespace LOFAR

#endif
