#ifndef LOFAR_CNPROC_TRANSPOSED_DATA_H
#define LOFAR_CNPROC_TRANSPOSED_DATA_H

#include <CoInterface/Config.h>
#include <CoInterface/Allocator.h>
#include <CoInterface/StreamableData.h>


namespace LOFAR
{
  namespace Cobalt
  {

    template <typename SAMPLE_TYPE>
    class TransposedData : public SampleData<SAMPLE_TYPE,3,1>
    {
    public:
      typedef SampleData<SAMPLE_TYPE,3,1> SuperType;

      TransposedData(const unsigned nrStations, const unsigned nrSamplesToCNProc, Allocator &allocator = heapAllocator);
    };


    template <typename SAMPLE_TYPE>
    inline TransposedData<SAMPLE_TYPE>::TransposedData(const unsigned nrStations, const unsigned nrSamplesToCNProc, Allocator &allocator)
      :
      SuperType(boost::extents[nrStations][nrSamplesToCNProc][NR_POLARIZATIONS], boost::extents[0], allocator)
    {
    }

  } // namespace Cobalt
} // namespace LOFAR

#endif
