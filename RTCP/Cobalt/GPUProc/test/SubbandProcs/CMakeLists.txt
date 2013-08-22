# $Id$

include(LofarCTest)

if(UNITTEST++_FOUND)
  lofar_add_test(tCorrelatorSubbandProc tCorrelatorSubbandProc.cc)
  lofar_add_test(tSubbandProc tSubbandProc.cc)
endif()

lofar_add_test(tCorrelatorSubbandProcProcessSb tCorrelatorSubbandProcProcessSb.cc)

# Trick tests that compile kernels that need to be installed into thinking
# that they can find the kernels using the LOFARROOT environment variable.
set_tests_properties(
  tCorrelatorSubbandProcProcessSb
  PROPERTIES ENVIRONMENT "LOFARROOT=${PACKAGE_SOURCE_DIR}"
)
