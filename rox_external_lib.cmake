function(rox_external_lib LIB_NAME LIB_VERSION LIB_URL LIB_MD5)

    string(REPLACE "<LIB_NAME>" ${LIB_NAME} LIB_URL1 ${LIB_URL})
    string(REPLACE "<LIB_VERSION>" ${LIB_VERSION} LIB_URL2 ${LIB_URL})

    if (ARGN)
        set(LIB_FILE ${ARGN})
    else ()
        set(LIB_FILE ${LIB_NAME}.lib)
    endif ()

    set(LIB_ROOT ${ROX_THIRD_PARTY_LIBS_LOCATION}/${LIB_NAME}/${LIB_VERSION})
    set(LIB_DOWNLOAD_DIR ${LIB_ROOT}/download)
    set(LIB_INSTALL_DIR ${LIB_ROOT}/install)
    set(LIB_SOURCE_DIR ${LIB_ROOT}/source)
    set(LIB_STAMP_DIR ${LIB_ROOT}/stamp)
    set(LIB_TMP_DIR ${LIB_ROOT}/tmp)
    set(LIB_FILE_LOCATION ${LIB_INSTALL_DIR}/lib/${LIB_FILE})

    if (EXISTS "${LIB_FILE_LOCATION}")

        add_library(${LIB_NAME} STATIC IMPORTED)
        set_target_properties(${LIB_NAME} PROPERTIES
                IMPORTED_LOCATION "${LIB_FILE_LOCATION}"
                INTERFACE_INCLUDE_DIRECTORIES "${LIB_INSTALL_DIR}/include")

    else ()

        if (ROX_BUILD_THIRD_PARTY_LIBS)

            # FIXME: ensure the same arch and toolset is used
            ExternalProject_Add(${LIB_NAME}_target
                    PREFIX ${LIB_VERSION}
                    URL ${LIB_URL2}
                    URL_MD5 ${LIB_MD5}
                    BUILD_IN_SOURCE 1
                    DOWNLOAD_DIR ${LIB_DOWNLOAD_DIR}
                    SOURCE_DIR ${LIB_SOURCE_DIR}
                    TMP_DIR ${LIB_TMP_DIR}
                    STAMP_DIR ${LIB_STAMP_DIR}
                    INSTALL_DIR ${LIB_INSTALL_DIR}
                    CONFIGURE_COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" -D CMAKE_INSTALL_PREFIX=<INSTALL_DIR> ./CMakeLists.txt)
        else ()

            message(FATAL_ERROR "${LIB_NAME} library not found in ${LIB_FILE_LOCATION}. Build third-party libs by navigating to vendor and calling ./build-third-party-libs.sh.")

        endif ()

    endif ()

endfunction()
