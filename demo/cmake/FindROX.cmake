if (NOT DEFINED ROX_SDK_ROOT)
    if (DEFINED $ENV{ROX_SDK_ROOT})
        set(ROX_SDK_ROOT $ENV{ROX_SDK_ROOT})
    endif ()
endif ()

if (NOT "${ROX_SDK_ROOT}" STREQUAL "")
    list(APPEND CMAKE_PREFIX_PATH ${ROX_SDK_ROOT})
else ()
    message(WARNING "ROX_SDK_ROOT is not defined. Looking inside system directories.")
endif ()

FIND_PATH(ROX_INCLUDE_DIR rox/server.h)
FIND_LIBRARY(ROX_LIBRARY NAMES rollout)
SET(ROX_TARGET_NAME ROX::SDK)

# Handle the QUIETLY and REQUIRED arguments and set ROX_FOUND to TRUE if all listed variables are TRUE.
INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(ROX REQUIRED_VARS ROX_LIBRARY ROX_INCLUDE_DIR)

# Copy the results to the output variables.
if (ROX_FOUND)
    SET(ROX_LIBRARIES ${ROX_LIBRARY})
    SET(ROX_INCLUDE_DIRS ${ROX_INCLUDE_DIR})
    if (NOT TARGET ${ROX_TARGET_NAME})
        add_library(${ROX_TARGET_NAME} UNKNOWN IMPORTED)
        set_target_properties(${ROX_TARGET_NAME} PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${ROX_INCLUDE_DIR}"
                IMPORTED_LOCATION "${ROX_LIBRARY}")
    endif ()
else (ROX_FOUND)
    SET(ROX_LIBRARIES)
    SET(ROX_INCLUDE_DIRS)
endif (ROX_FOUND)

MARK_AS_ADVANCED(ROX_INCLUDE_DIRS ROX_LIBRARIES)