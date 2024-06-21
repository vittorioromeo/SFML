#
# Try to find FLAC libraries and include paths.
# Once done this will define
#
# FLAC_FOUND
# FLAC_INCLUDE_DIR
# FLAC_LIBRARY
#

find_path(FLAC_INCLUDE_DIR FLAC/all.h)
find_path(FLAC_INCLUDE_DIR FLAC/stream_decoder.h)

find_library(FLAC_LIBRARY NAMES FLAC)

if (MINGW AND NOT SFML_USE_SYSTEM_DEPS AND SFML_COMPILER_GCC AND "${GCC_COMPILER_VERSION}" MATCHES "ucrt")
  string(REPLACE "FLAC" "FLAC-ucrt" FLAC_LIBRARY ${FLAC_LIBRARY})
  message(STATUS "FLAC REPLACEMENT DONE")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FLAC DEFAULT_MSG FLAC_LIBRARY FLAC_INCLUDE_DIR)

mark_as_advanced(FLAC_INCLUDE_DIR FLAC_LIBRARY)

add_library(FLAC::FLAC IMPORTED UNKNOWN)

target_link_libraries(FLAC::FLAC INTERFACE ws2_32)

set_target_properties(FLAC::FLAC PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${FLAC_INCLUDE_DIR})
if(FLAC_LIBRARY MATCHES "/([^/]+)\\.framework$")
    set_target_properties(FLAC::FLAC PROPERTIES IMPORTED_LOCATION ${FLAC_LIBRARY}/${CMAKE_MATCH_1})
else()
    set_target_properties(FLAC::FLAC PROPERTIES IMPORTED_LOCATION ${FLAC_LIBRARY})
endif()
