cmake_minimum_required(VERSION 3.0)
project (jsonc-daccord LANGUAGES C VERSION "0.5")

option(JDAC_RUN_TEST_SUITE     "Run JSON-Schema-org's test suite" OFF)
option(JDAC_RUN_TESTS          "Run tests" OFF)
option(JDAC_USE_ASAN           "Build with address-sanitizer" OFF)
option(JDAC_SHARED_LIBS        "Build .so file, .a otherwise" OFF)
option(JDAC_BUILD_ERROR_OUTPUT "Show errors neatly in schema validations" ON)
option(JDAC_BUILD_ALL          "Override individual BUILD options and build all" OFF)
option(JDAC_STATIC_ANALYZER    "Run static analysis" OFF)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -Wno-error=cpp -Wall -Wno-error=deprecated-declarations")
set(CMAKE_C_FLAGS_RELEASE "-Os -Werror")
set(CMAKE_C_FLAGS_DEBUG "-ggdb -O0 -fno-omit-frame-pointer")

if (NOT DEFINED CMAKE_INSTALL_LIBDIR)
  set(CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_PREFIX}/lib)
endif()

if(NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE Release)
endif()
string(TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE)

find_library(JSONC_LIBRARY NAMES libjson-c.so)
if (NOT JSONC_LIBRARY)
message(FATAL_ERROR "libjson-c.so not found!")
endif()

if (JDAC_STATIC_ANALYZER)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fanalyzer")
endif ()

configure_file("config.h.in" ${CMAKE_BINARY_DIR}/generated/version_config.h)
include_directories(${CMAKE_BINARY_DIR}/generated)

add_subdirectory(libjsoncdac)

if (JDAC_RUN_TESTS AND NOT (BUILD_TYPE STREQUAL "debug"))
  message(FATAL_ERROR "JDAC_RUN_TESTS needs CMAKE_BUILD_TYPE=debug")
endif ()

if (JDAC_RUN_TESTS)
  find_library(CMOCKA_LIBRARY NAMES libcmocka.so)
  if (NOT CMOCKA_LIBRARY)
    message(FATAL_ERROR "Could not find the cmocka library! Try: libcmocka-dev")
  endif()
  include(CTest)
  enable_testing()
  add_subdirectory(tests)
endif ()


if (UNIX OR MINGW OR CYGWIN)
  SET(prefix ${CMAKE_INSTALL_PREFIX})
  SET(exec_prefix ${CMAKE_INSTALL_PREFIX})
  SET(libdir ${CMAKE_INSTALL_PREFIX}/lib)
  SET(includedir ${CMAKE_INSTALL_PREFIX}/include)
  SET(VERSION ${PROJECT_VERSION})
	SET(LIBS_LIST ${JSONC_LIBRARY})

	function(list_transform_prepend var prefix)
		set(temp "")
		foreach(f ${${var}})
			list(APPEND temp "${prefix}${f}")
		endforeach()
		set(${var} "${temp}" PARENT_SCOPE)
	endfunction()
	list_transform_prepend(LIBS_LIST "-l")

	string(REPLACE ";" " " LIBS "${LIBS_LIST}")

  configure_file(jsonc-daccord.pc.in jsonc-daccord.pc @ONLY)
  set(INSTALL_PKGCONFIG_DIR "${CMAKE_INSTALL_LIBDIR}/pkgconfig" CACHE PATH "Installation directory for pkgconfig (.pc) files")
  install(FILES ${PROJECT_BINARY_DIR}/jsonc-daccord.pc DESTINATION "${INSTALL_PKGCONFIG_DIR}")
endif ()
