macro(rox_build_cmake_command VAR LIB_SOURCE_DIR LIB_BINARY_DIR)
    set(${VAR} "${CMAKE_COMMAND}" -S ${LIB_SOURCE_DIR} -B ${LIB_BINARY_DIR} -G "${CMAKE_GENERATOR}")
    if (CMAKE_GENERATOR_PLATFORM)
        set(${VAR} ${VAR} -A "${CMAKE_GENERATOR_PLATFORM}")
    endif ()
    if (CMAKE_GENERATOR_TOOLSET)
        set(${VAR} ${VAR} -T "${CMAKE_GENERATOR_TOOLSET}")
    endif ()
endmacro()

function(rox_external_lib LIB_NAME)

    set(options VERBOSE DRY_RUN)
    set(oneValueArgs VERSION URL HASH FILE CONFIGURE SUBDIR CMAKE)
    set(multiValueArgs)

    cmake_parse_arguments(LIB "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if (NOT LIB_VERSION)
        set(LIB_VERSION ${LIB_NAME})
    endif ()

    string(REPLACE "<LIB_NAME>" ${LIB_NAME} LIB_URL ${LIB_URL})
    string(REPLACE "<LIB_VERSION>" ${LIB_VERSION} LIB_URL ${LIB_URL})

    if (NOT LIB_FILE)
        set(LIB_FILE ${LIB_NAME})
    endif ()

    if (NOT LIB_FILE MATCHES "${CMAKE_STATIC_LIBRARY_SUFFIX}$")
        set(LIB_FILE "${LIB_FILE}${CMAKE_STATIC_LIBRARY_SUFFIX}")
    endif ()

    set(LIB_ROOT ${ROX_THIRD_PARTY_LIBS_LOCATION}/${LIB_NAME}/${LIB_VERSION})
    set(LIB_DOWNLOAD_DIR ${LIB_ROOT}/download)
    set(LIB_INSTALL_DIR ${LIB_ROOT}/install)
    set(LIB_SOURCE_DIR ${LIB_ROOT}/source)
    set(LIB_BINARY_DIR ${LIB_ROOT}/build)
    set(LIB_STAMP_DIR ${LIB_ROOT}/stamp)
    set(LIB_TMP_DIR ${LIB_ROOT}/tmp)

    if (NOT LIB_STATIC_DIR)
        set(LIB_STATIC_DIR "${LIB_INSTALL_DIR}/lib")
    endif ()

    set(LIB_FILE_LOCATION ${LIB_STATIC_DIR}/${LIB_FILE})

    if (NOT LIB_CMAKE)
        rox_build_cmake_command(LIB_CMAKE ${LIB_SOURCE_DIR}/${LIB_SUBDIR} ${LIB_BINARY_DIR})
    endif ()

    if (NOT LIB_CONFIGURE)
        set(LIB_CONFIGURE <CMAKE> -D CMAKE_INSTALL_PREFIX=${LIB_INSTALL_DIR})
    endif ()

    string(REPLACE "<CMAKE>" "${LIB_CMAKE}" LIB_CONFIGURE "${LIB_CONFIGURE}")

    if (LIB_VERBOSE)
        message("LIB_DRY_RUN = ${LIB_DRY_RUN}")
        message("LIB_URL = ${LIB_URL}")
        message("LIB_HASH = ${LIB_HASH}")
        message("LIB_FILE = ${LIB_FILE}")
        message("LIB_CONFIGURE = ${LIB_CONFIGURE}")
        message("LIB_SUBDIR = ${LIB_SUBDIR}")
        message("LIB_ROOT = ${LIB_ROOT}")
        message("LIB_DOWNLOAD_DIR = ${LIB_DOWNLOAD_DIR}")
        message("LIB_INSTALL_DIR = ${LIB_INSTALL_DIR}")
        message("LIB_SOURCE_DIR = ${LIB_SOURCE_DIR}")
        message("LIB_BINARY_DIR = ${LIB_BINARY_DIR}")
        message("LIB_STAMP_DIR = ${LIB_STAMP_DIR}")
        message("LIB_TMP_DIR = ${LIB_TMP_DIR}")
        message("LIB_FILE_LOCATION = ${LIB_FILE_LOCATION}")
        message("LIB_CMAKE = ${LIB_CMAKE}")
        message("LIB_CONFIGURE = ${LIB_CONFIGURE}")
    endif ()

    if (EXISTS "${LIB_FILE_LOCATION}")

        add_library(${LIB_NAME} STATIC IMPORTED)
        set_target_properties(${LIB_NAME} PROPERTIES
                IMPORTED_LOCATION "${LIB_FILE_LOCATION}"
                INTERFACE_INCLUDE_DIRECTORIES "${LIB_INSTALL_DIR}/include")

    else ()

        # TODO: check for library archive in the project source dir
        # TODO: check for library sources in the project source dir

        if (ROX_BUILD_THIRD_PARTY_LIBS)

            if (LIB_DRY_RUN)
                return()
            endif ()

            IF (LIB_URL MATCHES "\\.git$")

                ExternalProject_Add(${LIB_NAME}_target
                        PREFIX ${LIB_VERSION}
                        GIT_REPOSITORY ${LIB_URL}
                        GIT_TAG ${LIB_HASH}
                        SOURCE_DIR ${LIB_SOURCE_DIR}
                        SOURCE_SUBDIR ${LIB_SUBDIR}
                        TMP_DIR ${LIB_TMP_DIR}
                        STAMP_DIR ${LIB_STAMP_DIR}
                        INSTALL_DIR ${LIB_INSTALL_DIR}
                        BINARY_DIR ${LIB_BINARY_DIR}
                        CONFIGURE_COMMAND ${LIB_CONFIGURE})

            ELSE ()

                ExternalProject_Add(${LIB_NAME}_target
                        PREFIX ${LIB_VERSION}
                        URL ${LIB_URL}
                        URL_MD5 ${LIB_HASH}
                        DOWNLOAD_DIR ${LIB_DOWNLOAD_DIR}
                        SOURCE_DIR ${LIB_SOURCE_DIR}
                        SOURCE_SUBDIR ${LIB_SUBDIR}
                        TMP_DIR ${LIB_TMP_DIR}
                        STAMP_DIR ${LIB_STAMP_DIR}
                        INSTALL_DIR ${LIB_INSTALL_DIR}
                        BINARY_DIR ${LIB_BINARY_DIR}
                        CONFIGURE_COMMAND ${LIB_CONFIGURE})

            ENDIF ()


        else ()

            message(FATAL_ERROR "${LIB_NAME} library not found in ${LIB_FILE_LOCATION}. Build third-party libs by navigating to vendor and calling ./build-third-party-libs.sh.")

        endif ()

    endif ()

endfunction()
