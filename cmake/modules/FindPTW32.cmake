# - Find pthreads-win32
# Find the pthreads-win32 headers and libraries.
#
# PTW32_INCLUDE_DIR    - where to find pthread.h, etc.
# PTW32_LIBRARIES   - List of libraries when using pthreads-win32.
# PTW32_FOUND   - True if pthreads-win32 found.
# Look for the header file.
FIND_PATH(PTW32_INCLUDE_DIR pthread.h)
# Look for the library.
FIND_LIBRARY(PTW32_LIBRARY NAMES pthreadsVC2)
# Handle the QUIETLY and REQUIRED arguments and set PCRE_FOUND to TRUE if all listed variables are TRUE.
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(PTW32 REQUIRED_VARS PTW32_LIBRARY PTW32_INCLUDE_DIR)
# Copy the results to the output variables.
IF (PTW32_FOUND)
    SET(PTW32_LIBRARIES ${PTW32_LIBRARY})
    SET(PTW32_INCLUDE_DIRS ${PTW32_INCLUDE_DIR})
    if (NOT TARGET PTW32::PTW32)
        add_library(PTW32::PTW32 UNKNOWN IMPORTED)
        set_target_properties(PTW32::PTW32 PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${PTW32_INCLUDE_DIR}")
        set_property(TARGET PTW32::PTW32 APPEND PROPERTY IMPORTED_LOCATION "${PTW32_LIBRARY}")
    endif ()
ELSE (PTW32_FOUND)
    SET(PTW32_LIBRARIES)
    SET(PTW32_INCLUDE_DIRS)
ENDIF (PTW32_FOUND)
MARK_AS_ADVANCED(PTW32_INCLUDE_DIRS PTW32_LIBRARIES)