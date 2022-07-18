include_directories(../include)

set(jsoncdaccord_SOURCES validate.c)
set(HEADER_FILES ../include/jsoncdaccord.h)

add_library(jsoncdaccord_shared SHARED ${jsoncdaccord_SOURCES})
add_library(jsoncdaccord_static STATIC ${jsoncdaccord_SOURCES})

set_target_properties(
  jsoncdaccord_shared jsoncdaccord_static
  PROPERTIES  OUTPUT_NAME   jsoncdac
              PUBLIC_HEADER "${HEADER_FILES}"
)

#target_link_libraries(jsoncdaccord_static json-c)

install(
  TARGETS jsoncdaccord_shared jsoncdaccord_static
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  PUBLIC_HEADER DESTINATION include
)
