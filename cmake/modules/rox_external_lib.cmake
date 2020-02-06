macro(_rox_build_cmake_command VAR LIB_SOURCE_DIR)
    if (ARGN)
        set(${VAR} "${CMAKE_COMMAND}" -S ${LIB_SOURCE_DIR} -B ${ARGN} -G "${CMAKE_GENERATOR}")
    else ()
        set(${VAR} "${CMAKE_COMMAND}" -S ${LIB_SOURCE_DIR} -B ${LIB_SOURCE_DIR} -G "${CMAKE_GENERATOR}")
    endif ()
    if (CMAKE_GENERATOR_PLATFORM)
        set(${VAR} ${VAR} -A "${CMAKE_GENERATOR_PLATFORM}")
    endif ()
    if (CMAKE_GENERATOR_TOOLSET)
        set(${VAR} ${VAR} -T "${CMAKE_GENERATOR_TOOLSET}")
    endif ()
endmacro()

macro(_rox_build_lib_file_locations VAR)
    set(LIB_TARGET_FILE_NAMES "")
    list(LENGTH LIB_TARGETS TARGET_LIST_LEN)
    math(EXPR RANGE_END "${TARGET_LIST_LEN}-1")
    foreach (index RANGE 0 ${RANGE_END} 2)
        math(EXPR next_index "${index}+1")
        list(GET LIB_TARGETS ${index} LIB_TARGET_NAME)
        list(GET LIB_TARGETS ${next_index} LIB_TARGET_FILE_NAME)
        if (NOT LIB_TARGET_FILE_NAME MATCHES "${LIB_SUFFIX}$")
            set(LIB_TARGET_FILE_NAME "${LIB_TARGET_FILE_NAME}${LIB_SUFFIX}")
        endif ()
        if (LIB_SHARED)
            set(LIB_FILE_LOCATION ${LIB_SHARED_DIR}/${LIB_TARGET_FILE_NAME})
        else ()
            set(LIB_FILE_LOCATION ${LIB_STATIC_DIR}/${LIB_TARGET_FILE_NAME})
        endif ()
        list(APPEND LIB_TARGET_FILE_NAMES ${LIB_FILE_LOCATION})
    endforeach ()
    set(${VAR} ${LIB_TARGET_FILE_NAMES})
endmacro()

macro(_rox_prepare_lib_dependencies)
    set(LIB_DEPENDENCIES "")
    foreach (arg IN LISTS LIB_DEPENDS_ON)
        list(APPEND LIB_DEPENDENCIES build_${arg})
    endforeach ()
endmacro()

macro(_rox_init_third_party_lib_vars)

    if (LIB_CMAKE_ARGS)
        set(LIB_CMAKE_ARGS_PROCESSED "")
        foreach (arg IN LISTS LIB_CMAKE_ARGS)
            list(APPEND LIB_CMAKE_ARGS_PROCESSED "-D${arg}")
        endforeach ()
        set(LIB_CMAKE_ARGS "${LIB_CMAKE_ARGS_PROCESSED}")
    endif ()

    if (NOT LIB_VERSION)
        set(LIB_VERSION ${LIB_NAME})
    endif ()

    string(REPLACE "<LIB_NAME>" ${LIB_NAME} LIB_URL ${LIB_URL})
    string(REPLACE "<LIB_VERSION>" ${LIB_VERSION} LIB_URL ${LIB_URL})

    if (NOT LIB_FILE)
        set(LIB_FILE ${LIB_NAME})
    endif ()

    if (LIB_SHARED)
        set(LIB_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
    else ()
        set(LIB_SUFFIX ${CMAKE_STATIC_LIBRARY_SUFFIX})
    endif ()

    set(LIB_ROOT ${ROX_THIRD_PARTY_LIBS_LOCATION}/${LIB_NAME}/${LIB_VERSION})
    set(LIB_DOWNLOAD_DIR ${LIB_ROOT}/download)
    set(LIB_INSTALL_DIR ${LIB_ROOT}/install)
    set(LIB_SOURCE_DIR ${LIB_ROOT}/source)
    set(LIB_BINARY_DIR ${LIB_ROOT}/build)
    set(LIB_STAMP_DIR ${LIB_ROOT}/stamp)
    set(LIB_TMP_DIR ${LIB_ROOT}/tmp)

    if (EXISTS ${ROX_THIRD_PARTY_LIBS_CUSTOM_CONFIG_LOCATION}/${LIB_NAME}.cmake)
        include(${ROX_THIRD_PARTY_LIBS_CUSTOM_CONFIG_LOCATION}/${LIB_NAME}.cmake)
    endif ()

    if (LIB_BUILD_IN_SOURCE)
        set(LIB_BINARY_DIR)
    endif ()

    if (NOT LIB_STATIC_DIR)
        set(LIB_STATIC_DIR "${LIB_INSTALL_DIR}/lib")
    endif ()

    if (NOT LIB_SHARED_DIR)
        set(LIB_SHARED_DIR "${LIB_INSTALL_DIR}/bin")
    endif ()

    if (NOT LIB_CMAKE)
        _rox_build_cmake_command(LIB_CMAKE ${LIB_SOURCE_DIR}/${LIB_SUBDIR} ${LIB_BINARY_DIR})
    endif ()

    if (NOT LIB_CONFIGURE)
        set(LIB_CONFIGURE <CMAKE> -D CMAKE_INSTALL_PREFIX=${LIB_INSTALL_DIR} ${LIB_CMAKE_ARGS})
    endif ()

    if (NOT LIB_TARGETS)
        set(LIB_TARGETS ${LIB_NAME} ${LIB_FILE})
    endif ()

    _rox_build_lib_file_locations(LIB_FILE_LOCATIONS)
    _rox_prepare_lib_dependencies()

    string(REPLACE "<CMAKE>" "${LIB_CMAKE}" LIB_CONFIGURE "${LIB_CONFIGURE}")

    if (LIB_VERBOSE)
        message("LIB_NAME = ${LIB_NAME}")
        message("LIB_VERSION = ${LIB_VERSION}")
        message("LIB_URL = ${LIB_URL}")
        message("LIB_HASH = ${LIB_HASH}")
        message("LIB_FILE = ${LIB_FILE}")
        message("LIB_SUBDIR = ${LIB_SUBDIR}")
        message("LIB_ROOT = ${LIB_ROOT}")
        message("LIB_SHARED = ${LIB_SHARED}")
        message("LIB_DOWNLOAD_DIR = ${LIB_DOWNLOAD_DIR}")
        message("LIB_INSTALL_DIR = ${LIB_INSTALL_DIR}")
        message("LIB_SOURCE_DIR = ${LIB_SOURCE_DIR}")
        message("LIB_BINARY_DIR = ${LIB_BINARY_DIR}")
        message("LIB_STAMP_DIR = ${LIB_STAMP_DIR}")
        message("LIB_TMP_DIR = ${LIB_TMP_DIR}")
        message("LIB_CMAKE = ${LIB_CMAKE}")
        message("LIB_CMAKE_ARGS = ${LIB_CMAKE_ARGS}")
        message("LIB_CONFIGURE = ${LIB_CONFIGURE}")
        message("LIB_BUILD_IN_SOURCE = ${LIB_BUILD_IN_SOURCE}")
        message("LIB_DRY_RUN = ${LIB_DRY_RUN}")
        message("LIB_LINK = ${LIB_LINK}")
        message("LIB_TARGETS = ${LIB_TARGETS}")
        message("LIB_TRY_FIND = ${LIB_TRY_FIND}")
        message("LIB_TRY_FIND_THEN = ${LIB_TRY_FIND_THEN}")
        message("LIB_TRY_FIND_VERSION = ${LIB_TRY_FIND_VERSION}")
        message("LIB_TRY_FIND_IN_INSTALL_DIR = ${LIB_TRY_FIND_IN_INSTALL_DIR}")
        message("LIB_TRY_FIND_INCLUDE_DIR = ${LIB_TRY_FIND_INCLUDE_DIR}")
        message("LIB_FILE_LOCATIONS = ${LIB_FILE_LOCATIONS}")
        message("LIB_DEPENDENCIES = ${LIB_DEPENDENCIES}")
    endif ()

endmacro()

macro(_rox_check_if_all_file_locations_exist)
    foreach (arg IN LISTS LIB_FILE_LOCATIONS)
        if (LIB_VERBOSE)
            message("Checking for file existence: ${arg}")
        endif ()
        if (NOT EXISTS ${arg})
            if (LIB_VERBOSE)
                message("Library file ${arg} not found")
            endif ()
            set(LIB_FILE_NOT_FOUND 1)
        endif ()
    endforeach ()
endmacro()

macro(_rox_build_third_party_lib)

    _rox_check_if_all_file_locations_exist()

    if (NOT LIB_FILE_NOT_FOUND)

        if (LIB_VERBOSE)
            message("${LIB_NAME}: all library files exist; Nothing to build.")
        endif ()

    else ()

        if (LIB_VERBOSE)
            message("Building third party library ${LIB_NAME}")
        endif ()

        if (LIB_URL MATCHES "\\.git$")

            if (LIB_HASH)
                SET(GIT_TAG "${LIB_HASH}")
            elseif (LIB_VERSION)
                SET(GIT_TAG "${LIB_VERSION}")
            else ()
                SET(GIT_TAG "master")
            endif ()

            # TODO: check for library archive in the project source dir

            ExternalProject_Add(build_${LIB_NAME}
                    PREFIX ${LIB_VERSION}
                    GIT_REPOSITORY ${LIB_URL}
                    GIT_TAG ${GIT_TAG}
                    SOURCE_DIR ${LIB_SOURCE_DIR}
                    SOURCE_SUBDIR ${LIB_SUBDIR}
                    TMP_DIR ${LIB_TMP_DIR}
                    STAMP_DIR ${LIB_STAMP_DIR}
                    INSTALL_DIR ${LIB_INSTALL_DIR}
                    BINARY_DIR ${LIB_BINARY_DIR}
                    CONFIGURE_COMMAND ${LIB_CONFIGURE}
                    BUILD_IN_SOURCE ${LIB_BUILD_IN_SOURCE}
                    DEPENDS ${LIB_DEPENDENCIES})

        else ()

            ExternalProject_Add(build_${LIB_NAME}
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
                    CONFIGURE_COMMAND ${LIB_CONFIGURE}
                    BUILD_IN_SOURCE ${LIB_BUILD_IN_SOURCE}
                    DEPENDS ${LIB_DEPENDENCIES})

        endif ()

    endif ()

endmacro()

macro(_rox_try_find_third_party_lib)
    if (LIB_TRY_FIND)

        if (NOT LIB_TRY_FIND_THEN)
            set(LIB_TRY_FIND_THEN ${LIB_TRY_FIND}_FOUND)
        endif ()

        find_package(${LIB_TRY_FIND})

        if (NOT ${LIB_TRY_FIND_THEN} AND ${LIB_TRY_FIND_IN_INSTALL_DIR})
            set(${TRY_FIND_IN_INSTALL_DIR} ${LIB_INSTALL_DIR})
            find_package(${LIB_TRY_FIND})
        endif ()

        if (${LIB_TRY_FIND_THEN} AND ${LIB_TRY_FIND_INCLUDE_DIR})
            include_directories(${${LIB_TRY_FIND_INCLUDE_DIR}})
        endif ()

        # TODO: check lib version

    endif ()
endmacro()

macro(_rox_link_third_party_lib)

    list(LENGTH LIB_TARGETS TARGET_LIST_LEN)
    math(EXPR RANGE_END "${TARGET_LIST_LEN}/2-1")

    foreach (index RANGE 0 ${RANGE_END})

        math(EXPR lib_target_name_index "${index}*2")
        math(EXPR lib_file_name_index "${index}*2+1")
        list(GET LIB_TARGETS ${lib_target_name_index} LIB_TARGET_NAME)
        list(GET LIB_TARGETS ${lib_file_name_index} LIB_TARGET_FILE_NAME)

        if (NOT TARGET ${LIB_TARGET_NAME})

            list(GET LIB_FILE_LOCATIONS ${index} LIB_FILE_LOCATION)

            if (EXISTS "${LIB_FILE_LOCATION}")

                if (LIB_SHARED)
                    add_library(${LIB_TARGET_NAME} SHARED IMPORTED)
                else ()
                    add_library(${LIB_TARGET_NAME} STATIC IMPORTED)
                endif ()

                set_target_properties(${LIB_TARGET_NAME} PROPERTIES
                        IMPORTED_LOCATION "${LIB_FILE_LOCATION}"
                        INTERFACE_INCLUDE_DIRECTORIES "${LIB_INSTALL_DIR}/include")

            else ()

                message(FATAL_ERROR "${LIB_NAME} library not found in ${LIB_FILE_LOCATION}. Build third-party libs by navigating to vendor and calling ./build-third-party-libs.sh.")

            endif ()

        endif ()

        if (LIB_LINK)
            set_target_properties(${LIB_TARGET_NAME} PROPERTIES IMPORTED_LINK_DEPENDENT_LIBRARIES "${LIB_LINK}")
        endif ()

        list(APPEND ROX_EXTERNAL_LIBS ${LIB_TARGET_NAME})

    endforeach ()
endmacro()

function(rox_external_lib LIB_NAME)

    set(options VERBOSE DRY_RUN SHARED BUILD_IN_SOURCE)
    set(oneValueArgs VERSION URL HASH FILE CONFIGURE SUBDIR CMAKE TRY_FIND TRY_FIND_THEN TRY_FIND_VERSION TRY_FIND_INCLUDE_DIR TRY_FIND_IN_INSTALL_DIR)
    set(multiValueArgs CMAKE_ARGS TARGETS LINK DEPENDS_ON)

    cmake_parse_arguments(LIB "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(ROX_EXTERNAL_LIBS ${ROX_EXTERNAL_LIBS})

    _rox_init_third_party_lib_vars()
    _rox_try_find_third_party_lib()

    if (ROX_BUILD_THIRD_PARTY_LIBS)

        if (LIB_DRY_RUN)
            return()
        endif ()

        _rox_build_third_party_lib()

    else ()

        _rox_link_third_party_lib()

    endif ()

    set(ROX_EXTERNAL_LIBS ${ROX_EXTERNAL_LIBS} PARENT_SCOPE)

endfunction()
