cmake_minimum_required(VERSION 3.0)
project (jsonc-daccord VERSION "0.2")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -Wno-error=cpp -Wall")
set(CMAKE_C_FLAGS_RELEASE "-Os -Werror")
set(CMAKE_C_FLAGS_DEBUG "-ggdb -O0 -fsanitize=address -static-libasan -fno-omit-frame-pointer")

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()
string(TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE)

find_library(JSONC_LIBRARY NAMES libjson-c.so)
if (NOT JSONC_LIBRARY)
  message(FATAL_ERROR "libjson-c.so not found!")
endif()

configure_file("config.h.in" ${CMAKE_BINARY_DIR}/generated/version_config.h)
include_directories(${CMAKE_BINARY_DIR}/generated)

add_subdirectory(libjsoncdac)

add_executable(jdac-cli jdac-cli.c)

if (BUILD_TYPE STREQUAL "debug")
  find_library(CMOCKA_LIBRARY NAMES libcmocka.so)
  if (NOT CMOCKA_LIBRARY)
    message(FATAL_ERROR "Could not find the cmocka library! Try: libcmocka-dev")
  endif()
  target_link_libraries(jdac-cli PUBLIC jsoncdaccord_static json-c cmocka)
  include(CTest)
  enable_testing()
  add_subdirectory(tests)
else ()
  target_link_libraries(jdac-cli PUBLIC jsoncdaccord_shared json-c)
endif ()

install(TARGETS jdac-cli DESTINATION bin)
