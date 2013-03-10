#include <lofar_config.h>
#include "SampleBuffer.h"
#include "SampleType.h"
#include <Common/LofarLogger.h>
#include <vector>
#include <string>

using namespace LOFAR;
using namespace RTCP;
using namespace std;

template<typename T> void test( struct BufferSettings &settings )
{
  // Should be able to create the buffer
  SampleBuffer< SampleType<T> > buffer_create(settings, true);

  // Should be able to attach to created buffer
  SampleBuffer< SampleType<T> > buffer_read(settings, false);
}

int main() {
  INIT_LOGGER( "tSampleBuffer" );

  // Don't run forever if communication fails for some reason
  alarm(10);

  // Fill a BufferSettings object
  struct StationID stationID("RS106", "LBA", 200, 16);
  struct BufferSettings settings;

  settings.station = stationID;
  settings.nrBeamletsPerBoard = 61;
  settings.nrBoards = 1;

  settings.nrSamples = (2 * stationID.clockMHz * 1000000 / 1024);// & ~0xFL;
  settings.nrFlagRanges = 64;

  settings.dataKey = 0x12345678;

  // Test various modes
  LOG_INFO("Test 16-bit complex");
  test<i16complex>(settings);

  LOG_INFO("Test 8-bit complex");
  test<i8complex>(settings);

  LOG_INFO("Test 4-bit complex");
  test<i4complex>(settings);

  return 0;
}

