include_directories(../include)

option (BUILD_STRING_PATTERN "" ON)

set(jsoncdaccord_SOURCES validate.c)
set(PUBLIC_HEADER_FILES ../include/jsoncdaccord.h)

# always supported
list(APPEND SUPPORTED_BASE type enum required properties anyOf minLength maxLength minimum maximum)
list(APPEND SUPPORTED_BASE minItems maxItems uniqueItems items)

if (BUILD_STRING_PATTERN)
  set(jsoncdaccord_SOURCES validate.c string-pattern.c )
  list(APPEND SUPPORTED_OPTIONAL "patterns")
endif ()

string (REPLACE ";" "," SUPPORTED_BASE_STR "${SUPPORTED_BASE}")
string (REPLACE ";" "," SUPPORTED_OPTIONAL_STR "${SUPPORTED_OPTIONAL}")

add_library(jsoncdaccord_shared SHARED ${jsoncdaccord_SOURCES})
add_library(jsoncdaccord_static STATIC ${jsoncdaccord_SOURCES})
target_compile_definitions(jsoncdaccord_shared PRIVATE "SUPPORTED_KEYWORDS_BASE=\"${SUPPORTED_BASE_STR}\"" "SUPPORTED_KEYWORDS_OPTIONAL=\"${SUPPORTED_OPTIONAL_STR}\"")
target_compile_definitions(jsoncdaccord_static PRIVATE "SUPPORTED_KEYWORDS_BASE=\"${SUPPORTED_BASE_STR}\"" "SUPPORTED_KEYWORDS_OPTIONAL=\"${SUPPORTED_OPTIONAL_STR}\"")

set_target_properties(
  jsoncdaccord_shared jsoncdaccord_static
  PROPERTIES
    OUTPUT_NAME   jsoncdac
    PUBLIC_HEADER "${HEADER_FILES}"
    VERSION 1.0.0
    SOVERSION 1
)

install(
  TARGETS jsoncdaccord_shared jsoncdaccord_static
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  PUBLIC_HEADER DESTINATION include
)

add_executable(jdac-cli jdac-cli.c)
if (BUILD_TYPE STREQUAL "debug")
  target_link_libraries(jdac-cli PUBLIC jsoncdaccord_static json-c cmocka)
else ()
  target_link_libraries(jdac-cli PUBLIC jsoncdaccord_shared json-c)
endif ()
target_compile_definitions(jdac-cli PRIVATE "SUPPORTED_KEYWORDS_BASE=\"${SUPPORTED_BASE_STR}\"" "SUPPORTED_KEYWORDS_OPTIONAL=\"${SUPPORTED_OPTIONAL_STR}\"")
install(TARGETS jdac-cli DESTINATION bin)
