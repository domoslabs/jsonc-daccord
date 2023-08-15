include_directories(../include)

set(jsoncdaccord_SOURCES validate.c output.c)
set(PUBLIC_HEADER_FILES ../include/jsoncdaccord.h)

# plugin options
# The option name is the name of the c.file excluding the .c extension

list(APPEND OPTIONAL_ON_DEFAULT
  additionalproperties
  propertynames
  subschemalogic
  dependent
  contains
  ref
  store
)

list(APPEND OPTIONAL_OFF_DEFAULT
  pattern
  patternproperties
  download
)

foreach(src ${OPTIONAL_ON_DEFAULT})
  string(TOUPPER ${src} srcupper)
  option(BUILD_${srcupper} "${src} option" ON)
endforeach()

foreach(src ${OPTIONAL_OFF_DEFAULT})
  string(TOUPPER ${src} srcupper)
  option(BUILD_${srcupper} "${src} option" OFF)
endforeach()

# add selected plugins to source, and set definitions
foreach(src ${OPTIONAL_ON_DEFAULT} ${OPTIONAL_OFF_DEFAULT})
  string(TOUPPER ${src} srcupper)
  if (BUILD_${srcupper})
    list(APPEND SUPPORTED_OPTIONAL ${src})
    set(jsoncdaccord_SOURCES ${jsoncdaccord_SOURCES} ${src}.c )
    add_definitions(-DJDAC_${srcupper})
  endif()
endforeach()

if (BUILD_REF AND NOT BUILD_STORE)
  message(FATAL_ERROR "Can not have BUILD_REF without BUILD_STORE option")
endif()

if (BUILD_ERROR_OUTPUT)
  add_definitions(-DJDAC_ERROR_OUTPUT)
endif ()

if (BUILD_DOWNLOAD)
  find_package(CURL REQUIRED)
  include_directories(${CURL_INCLUDE_DIR})
  set(EXTRA_LIBS ${CURL_LIBRARIES})
endif()


set(EXTRA_LIBS ${EXTRA_LIBS} m json-c)
if (USE_ASAN)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -static-libasan -fno-omit-frame-pointer")
  set(EXTRA_LIBS ${EXTRA_LIBS} -fsanitize=address)
endif ()


if (BUILD_PATTERNPROPERTIES OR BUILD_PATTERN)
  set(jsoncdaccord_SOURCES ${jsoncdaccord_SOURCES} regex_match.c )
  #set(EXTRA_LIBS regex)
endif()

# always supported
list(APPEND SUPPORTED_BASE type enum required properties anyOf minLength maxLength minimum maximum const)
list(APPEND SUPPORTED_BASE minItems maxItems uniqueItems items)
string (REPLACE ";" "," SUPPORTED_BASE_STR "${SUPPORTED_BASE}")
string (REPLACE ";" "," SUPPORTED_OPTIONAL_STR "${SUPPORTED_OPTIONAL}")


if (JDAC_SHARED_LIBS)
  add_library(jsoncdaccord_shared SHARED ${jsoncdaccord_SOURCES})
  target_compile_definitions(jsoncdaccord_shared PRIVATE "SUPPORTED_KEYWORDS_BASE=\"${SUPPORTED_BASE_STR}\"" "SUPPORTED_KEYWORDS_OPTIONAL=\"${SUPPORTED_OPTIONAL_STR}\"")
  target_link_libraries(jsoncdaccord_shared ${EXTRA_LIBS})
  set_target_properties(jsoncdaccord_shared PROPERTIES OUTPUT_NAME jsoncdac PUBLIC_HEADER "${HEADER_FILES}" VERSION 1.0.0 SOVERSION 1 )
  install(
    TARGETS jsoncdaccord_shared
    EXPORT jsoncdaccordtarget
    # ARCHIVE DESTINATION lib
    # LIBRARY DESTINATION lib
    # PUBLIC_HEADER DESTINATION include
  )
  install(
    EXPORT jsoncdaccordtarget
    FILE jsoncdaccord.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake
  )
else ()
  add_library(jsoncdaccord_static STATIC ${jsoncdaccord_SOURCES})
  target_compile_definitions(jsoncdaccord_static PRIVATE "SUPPORTED_KEYWORDS_BASE=\"${SUPPORTED_BASE_STR}\"" "SUPPORTED_KEYWORDS_OPTIONAL=\"${SUPPORTED_OPTIONAL_STR}\"")
  target_link_libraries(jsoncdaccord_static ${EXTRA_LIBS})
  set_target_properties(jsoncdaccord_static PROPERTIES OUTPUT_NAME jsoncdac PUBLIC_HEADER "${HEADER_FILES}" VERSION 1.0.0 SOVERSION 1 )
  install(
    TARGETS jsoncdaccord_static
    EXPORT jsoncdaccordtarget
    # ARCHIVE DESTINATION lib
    # LIBRARY DESTINATION lib
    # PUBLIC_HEADER DESTINATION include
  )
  install(
    EXPORT jsoncdaccordtarget
    FILE jsoncdaccord.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake
  )
endif ()

if (JDAC_SHARED_LIBS)
  set(EXTRA_LIBS ${EXTRA_LIBS} jsoncdaccord_shared)
else ()
  set(EXTRA_LIBS ${EXTRA_LIBS} jsoncdaccord_static)
endif ()

add_executable(jdac-cli jdac-cli.c)
target_link_libraries(jdac-cli PUBLIC ${EXTRA_LIBS})
target_compile_definitions(jdac-cli PRIVATE "SUPPORTED_KEYWORDS_BASE=\"${SUPPORTED_BASE_STR}\"" "SUPPORTED_KEYWORDS_OPTIONAL=\"${SUPPORTED_OPTIONAL_STR}\"")
install(
  TARGETS jdac-cli
  RUNTIME DESTINATION bin
)
