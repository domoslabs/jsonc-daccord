include_directories(../include)

function(ADDTEST test_src)
  set(option_args "")
  set(single_value_arguments "")
  set(multi_value_arguments "SOURCES;LIBS;WRAPFUNCTIONS")
  cmake_parse_arguments(
    PARSE_ARGV 1 ADDTEST "${options_args}" "${single_value_arguments}" "${multi_value_arguments}"
  )
  get_filename_component(test_name ${test_src} NAME_WE)
  add_executable(${test_name} ${test_src} ${ADDTEST_SOURCES})
  target_link_libraries(${test_name} ${ADDTEST_LIBS})
  target_include_directories(${test_name} PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
  if (ADDTEST_WRAPFUNCTIONS)
    string (REPLACE ";" ",--wrap=" ADDTEST_WRAPFUNCTIONS "${ADDTEST_WRAPFUNCTIONS}")
    set_target_properties(${test_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests LINK_FLAGS -Wl,--wrap=${ADDTEST_WRAPFUNCTIONS})
  else()
    set_target_properties(${test_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests)
  endif()
  add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

file(COPY json schema DESTINATION ${CMAKE_BINARY_DIR}/tests)

addtest(test-flows.c    SOURCES "" LIBS cmocka json-c jsoncdaccord_static )
addtest(test-validate.c SOURCES "" LIBS cmocka json-c jsoncdaccord_static WRAPFUNCTIONS json_object_from_file )
