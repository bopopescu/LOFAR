# $Id$

include(LofarCTest)

set(_tests
  tCommensal_3sec_2st_3sb
)

foreach(_test ${_tests})
  lofar_add_test(${_test})
  if(BUILD_TESTING)
    set_tests_properties(${_test} PROPERTIES RUN_SERIAL ON)
  endif(BUILD_TESTING)
endforeach(_test ${_tests})
