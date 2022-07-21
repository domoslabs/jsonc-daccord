cmake_minimum_required(VERSION 3.0)
project (jsonc-daccord LANGUAGES C VERSION "0.1.99")

option(RUN_TEST_SUITS "Run JSON-Schema-org's test suits" OFF)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -Wno-error=cpp -Wall")
set(CMAKE_C_FLAGS_RELEASE "-Os -Werror")
set(CMAKE_C_FLAGS_DEBUG "-ggdb -O0 -fsanitize=address -static-libasan -fno-omit-frame-pointer")

include(FindPkgConfig)
pkg_check_modules (JSONC_LIBRARY REQUIRED json-c>=0.13)

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

if (BUILD_TYPE STREQUAL "debug")
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
	SET(LIBS_LIST json-c)

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
  set(INSTALL_PKGCONFIG_DIR "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" CACHE PATH "Installation directory for pkgconfig (.pc) files")
  install(FILES ${PROJECT_BINARY_DIR}/jsonc-daccord.pc DESTINATION "${INSTALL_PKGCONFIG_DIR}")
endif ()
