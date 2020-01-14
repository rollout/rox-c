cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeModules/")

include(ExternalProject)

function(external_lib LIB_NAME LIB_VERSION LIB_URL LIB_MD5)

    if (ARGN)
        set(LIB_FILE ${ARGN})
    else ()
        set(LIB_FILE ${LIB_NAME}.lib)
    endif ()

    string(REPLACE "<LIB_NAME>" ${LIB_NAME} LIB_URL1 ${LIB_URL})
    string(REPLACE "<LIB_VERSION>" ${LIB_VERSION} LIB_URL2 ${LIB_URL})

    set(LIB_ROOT ${CMAKE_CURRENT_BINARY_DIR}/third-party/${LIB_NAME}/${LIB_VERSION})
    set(LIB_DOWNLOAD_DIR ${LIB_ROOT}/download)
    set(LIB_INSTALL_DIR ${LIB_ROOT}/install)
    set(LIB_SOURCE_DIR ${LIB_ROOT}/source)
    set(LIB_STAMP_DIR ${LIB_ROOT}/stamp)
    set(LIB_TMP_DIR ${LIB_ROOT}/tmp)

    if (ROX_BUILD_THIRD_PARTY_LIBS)

        message(STATUS "Building lib ${LIB_NAME}")

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
                CONFIGURE_COMMAND cmake -G "${CMAKE_GENERATOR}" -D CMAKE_INSTALL_PREFIX=<INSTALL_DIR> ./CMakeLists.txt)

    else ()

        message(STATUS "Add imported library ${LIB_NAME} (${LIB_INSTALL_DIR}/lib/${LIB_FILE})")

        add_library(${LIB_NAME} STATIC IMPORTED)
        set_target_properties(${LIB_NAME} PROPERTIES
                IMPORTED_LOCATION "${LIB_INSTALL_DIR}/lib/${LIB_FILE}"
                INTERFACE_INCLUDE_DIRECTORIES "${LIB_INSTALL_DIR}/include")

    endif ()

endfunction()

project(rox-api)

enable_testing()

include_directories(include)

add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(tests)
