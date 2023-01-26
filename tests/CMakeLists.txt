include_directories(../include)

function(ADDTEST test_src)
  set(option_args "")
  set(single_value_arguments "")
  set(multi_value_arguments "TESTNAME;SOURCES;LIBS;WRAPFUNCTIONS;RUNNERARGS")
  cmake_parse_arguments(
    PARSE_ARGV 1 ADDTEST "${options_args}" "${single_value_arguments}" "${multi_value_arguments}"
  )

  if (ADDTEST_TESTNAME)
    set(test_name ${ADDTEST_TESTNAME})
  else()
    get_filename_component(test_name ${test_src} NAME_WE)
  endif()

  add_executable(${test_name} ${test_src} ${ADDTEST_SOURCES})
  target_link_libraries(${test_name} PUBLIC ${ADDTEST_LIBS})
  target_include_directories(${test_name} PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
  if (ADDTEST_WRAPFUNCTIONS)
    string (REPLACE ";" ",--wrap=" ADDTEST_WRAPFUNCTIONS "${ADDTEST_WRAPFUNCTIONS}")
    set_target_properties(${test_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests LINK_FLAGS -Wl,--wrap=${ADDTEST_WRAPFUNCTIONS})
  else()
    set_target_properties(${test_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests)
  endif()
  add_test(NAME ${test_name} COMMAND ${test_name} ${ADDTEST_RUNNERARGS})
endfunction()

file(COPY json schema DESTINATION ${CMAKE_BINARY_DIR}/tests)

addtest(test-flows.c    SOURCES "" LIBS jsoncdaccord_static cmocka json-c  )
addtest(test-validate.c SOURCES "" LIBS cmocka json-c jsoncdaccord_static WRAPFUNCTIONS json_object_from_file )
addtest(test-suite-side-tests.c SOURCES "" LIBS cmocka json-c jsoncdaccord_static )
addtest(test-dependent.c SOURCES "" LIBS cmocka json-c jsoncdaccord_static )

# Test JSON Schema Test Suite
if (RUN_TEST_SUITE)
  file(GLOB DRAFT2020_12          RELATIVE "${PROJECT_SOURCE_DIR}/tests/JSON-Schema-Test-Suite/tests/" "${PROJECT_SOURCE_DIR}/tests/JSON-Schema-Test-Suite/tests/draft2020-12/*.json")
  #file(GLOB DRAFT2020_12_OPTIONAL RELATIVE "${PROJECT_SOURCE_DIR}/tests/JSON-Schema-Test-Suite/tests/" "${PROJECT_SOURCE_DIR}/tests/JSON-Schema-Test-Suite/tests/draft2020-12/optional/*.json")
  #file(GLOB DRAFT2020_12_FORMAT   RELATIVE "${PROJECT_SOURCE_DIR}/tests/JSON-Schema-Test-Suite/tests/" "${PROJECT_SOURCE_DIR}/tests/JSON-Schema-Test-Suite/tests/draft2020-12/optional/format/*.json")
  set(DRAFT2020_12 ${DRAFT2020_12} ${DRAFT2020_12_OPTIONAL} ${DRAFT2020_12_FORMAT})

  file(COPY JSON-Schema-Test-Suite DESTINATION ${CMAKE_BINARY_DIR}/tests)

  foreach(ITEM ${DRAFT2020_12})
    get_filename_component(test_name ${ITEM} NAME_WE)
    addtest(test_runner.c TESTNAME ${test_name} LIBS cmocka json-c jsoncdaccord_shared RUNNERARGS ${CMAKE_BINARY_DIR}/tests/JSON-Schema-Test-Suite/tests/${ITEM})
  endforeach()
endif ()
