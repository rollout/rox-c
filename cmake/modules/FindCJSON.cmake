# - Find cJSON
# Find the native cJSON headers and libraries.
#
# CJSON_INCLUDE_DIRS    - where to find cJSON.h, etc.
# CJSON_LIBRARIES   - List of libraries when using cJSON.
# CJSON_FOUND   - True if cJSON found.
# Look for the header file.
FIND_PATH(CJSON_INCLUDE_DIR cjson/cjson.h)
# Look for the library.
if (CJSON_STATIC)
    FIND_LIBRARY(CJSON_LIBRARY NAMES libcjson)
else ()
    FIND_LIBRARY(CJSON_LIBRARY NAMES cjson)
endif ()

# Handle the QUIETLY and REQUIRED arguments and set CJSON_FOUND to TRUE if all listed variables are TRUE.
INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(CJSON REQUIRED_VARS CJSON_LIBRARY CJSON_INCLUDE_DIR)

# Copy the results to the output variables.
if (CJSON_FOUND)
    SET(CJSON_LIBRARIES ${CJSON_LIBRARY})
    SET(CJSON_INCLUDE_DIRS ${CJSON_INCLUDE_DIR})
    if (NOT TARGET CJSON::CJSON)
        add_library(CJSON::CJSON UNKNOWN IMPORTED)
        set_target_properties(CJSON::CJSON PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${CJSON_INCLUDE_DIR}")
        set_property(TARGET CJSON::CJSON APPEND PROPERTY IMPORTED_LOCATION "${CJSON_LIBRARY}")
    endif ()
else (CJSON_FOUND)
    SET(CJSON_LIBRARIES)
    SET(CJSON_INCLUDE_DIRS)
endif (CJSON_FOUND)
MARK_AS_ADVANCED(CJSON_INCLUDE_DIRS CJSON_LIBRARIES)