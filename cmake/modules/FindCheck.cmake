# - Find Check
# Find the native Check headers and libraries.
#
# CHECK_INCLUDE_DIRS    - where to find check.h, etc.
# CHECK_LIBRARIES   - List of libraries when using Check.
# CHECK_FOUND   - True if Check found.
# Look for the header file.
FIND_PATH(CHECK_INCLUDE_DIR check.h)

if (CHECK_STATIC)
    set(CHECK_LIBRARY_NAME ${CMAKE_STATIC_LIBRARY_PREFIX}check${CMAKE_STATIC_LIBRARY_SUFFIX})
else ()
    set(CHECK_LIBRARY_NAME ${CMAKE_SHARED_LIBRARY_PREFIX}checkDynamic${CMAKE_SHARED_LIBRARY_SUFFIX})
endif ()

# Look for the library.
FIND_LIBRARY(CHECK_LIBRARY NAMES ${CHECK_LIBRARY_NAME})

# Handle the QUIETLY and REQUIRED arguments and set CHECK_FOUND to TRUE if all listed variables are TRUE.
INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(CHECK REQUIRED_VARS CHECK_LIBRARY CHECK_INCLUDE_DIR)

# Copy the results to the output variables.
IF (CHECK_FOUND)
    SET(CHECK_LIBRARIES ${CHECK_LIBRARY})
    SET(CHECK_INCLUDE_DIRS ${CHECK_INCLUDE_DIR})
    if (NOT TARGET Check::Check)
        add_library(Check::Check UNKNOWN IMPORTED)
        set_target_properties(Check::Check PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${CHECK_INCLUDE_DIR}")
        set_property(TARGET Check::Check APPEND PROPERTY IMPORTED_LOCATION "${CHECK_LIBRARY}")
    endif ()
ELSE (CHECK_FOUND)
    SET(CHECK_LIBRARIES)
    SET(CHECK_INCLUDE_DIRS)
ENDIF (CHECK_FOUND)
MARK_AS_ADVANCED(CHECK_INCLUDE_DIRS CHECK_LIBRARIES)