macro(_rox_build_cmake_command VAR LIB_SOURCE_DIR)
    if (ARGN)
        set(${VAR} "${CMAKE_COMMAND}" -S ${LIB_SOURCE_DIR} -B ${ARGN} -G "${CMAKE_GENERATOR}")
    else ()
        set(${VAR} "${CMAKE_COMMAND}" -S ${LIB_SOURCE_DIR} -B ${LIB_SOURCE_DIR} -G "${CMAKE_GENERATOR}")
    endif ()
    if (CMAKE_GENERATOR_PLATFORM)
        set(${VAR} "${VAR} -A \"${CMAKE_GENERATOR_PLATFORM}\"")
    endif ()
    if (CMAKE_GENERATOR_TOOLSET)
        set(${VAR} "${VAR} -T \"${CMAKE_GENERATOR_TOOLSET}\"")
    endif ()
endmacro()

macro(_rox_build_lib_file_locations VAR)
    if (LIB_TARGETS)
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
    endif ()
endmacro()

macro(_rox_prepare_lib_dependencies)
    set(LIB_DEPENDENCIES "")
    foreach (arg IN LISTS LIB_DEPENDS_ON)
        if (TARGET lib_${arg})
            list(APPEND LIB_DEPENDENCIES lib_${arg})
        endif ()
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

    if (LIB_TRY_FIND_IN_INSTALL_DIR_SET AND NOT LIB_TRY_FIND_IN_INSTALL_DIR)
        set(LIB_TRY_FIND_IN_INSTALL_DIR ON)
    endif ()

    list(LENGTH LIB_TRY_FIND LIB_TRY_FIND_LEN)
    if (LIB_TRY_FIND_LEN GREATER 0)
        list(GET LIB_TRY_FIND 0 LIB_TRY_FIND_PACKAGE_NAME)
        if (LIB_TRY_FIND_LEN GREATER 1)
            list(SUBLIST LIB_TRY_FIND 1 -1 LIB_TRY_FIND_TARGET_NAMES)
        endif ()
    endif ()

    if (NOT LIB_TARGETS AND NOT LIB_TRY_FIND_LIBRARIES AND NOT LIB_TRY_FIND_TARGET_NAMES)
        set(LIB_TARGETS ${LIB_NAME} ${LIB_FILE})
    endif ()

    if (LIB_SHARED)
        set(LIB_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
    endif ()

    if (LIB_STATIC)
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
        set(LIB_CONFIGURE <CMAKE> -DCMAKE_INSTALL_PREFIX=${LIB_INSTALL_DIR} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} ${LIB_CMAKE_ARGS})
    endif ()

    if (NOT LIB_BUILD)
        set(LIB_BUILD ${CMAKE_MAKE_PROGRAM})
    endif ()

    if (LIB_CFLAGS)
        set(LIB_BUILD "${LIB_BUILD} CFLAGS=\"${LIB_CFLAGS}\"")
    endif ()

    _rox_build_lib_file_locations(LIB_FILE_LOCATIONS)
    _rox_prepare_lib_dependencies()

    string(REPLACE "<DEFAULT_BUILD>" "${CMAKE_MAKE_PROGRAM}" LIB_BUILD "${LIB_BUILD}")
    string(REPLACE "<DEFAULT_CONFIGURE>" "<CMAKE> -D CMAKE_INSTALL_PREFIX=${LIB_INSTALL_DIR} ${LIB_CMAKE_ARGS}" LIB_CONFIGURE "${LIB_CONFIGURE}")
    string(REPLACE "<CMAKE>" "${LIB_CMAKE}" LIB_CONFIGURE "${LIB_CONFIGURE}")

    if (WIN32 AND LIB_LINK_WIN)
        list(APPEND LIB_LINK ${LIB_LINK_WIN})
    endif ()

    if (NOT LIB_INCLUDE_DIR)
        set(LIB_INCLUDE_DIR ${LIB_INSTALL_DIR}/include)
    endif ()

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
        message("LIB_INCLUDE_DIR = ${LIB_INCLUDE_DIR}")
        message("LIB_CMAKE = ${LIB_CMAKE}")
        message("LIB_CMAKE_ARGS = ${LIB_CMAKE_ARGS}")
        message("LIB_CFLAGS = ${LIB_CFLAGS}")
        message("LIB_BUILD = ${LIB_BUILD}")
        message("LIB_PATCH = ${LIB_PATCH}")
        message("LIB_CONFIGURE = ${LIB_CONFIGURE}")
        message("LIB_BUILD_IN_SOURCE = ${LIB_BUILD_IN_SOURCE}")
        message("LIB_DRY_RUN = ${LIB_DRY_RUN}")
        message("LIB_LINK = ${LIB_LINK}")
        message("LIB_LINK_WIN = ${LIB_LINK_WIN}")
        message("LIB_TARGETS = ${LIB_TARGETS}")
        message("LIB_TRY_FIND = ${LIB_TRY_FIND}")
        message("LIB_TRY_FIND_PACKAGE_NAME = ${LIB_TRY_FIND_PACKAGE_NAME}")
        message("LIB_TRY_FIND_TARGET_NAMES = ${LIB_TRY_FIND_TARGET_NAMES}")
        message("LIB_TRY_FIND_THEN = ${LIB_TRY_FIND_THEN}")
        message("LIB_TRY_FIND_VERSION = ${LIB_TRY_FIND_VERSION}")
        message("LIB_TRY_FIND_IN = ${LIB_TRY_FIND_IN}")
        message("LIB_TRY_FIND_IN_INSTALL_DIR = ${LIB_TRY_FIND_IN_INSTALL_DIR}")
        message("LIB_TRY_FIND_IN_INSTALL_DIR_SET = ${LIB_TRY_FIND_IN_INSTALL_DIR_SET}")
        message("LIB_TRY_FIND_INCLUDE_DIR = ${LIB_TRY_FIND_INCLUDE_DIR}")
        message("LIB_TRY_FIND_LIBRARIES = ${LIB_TRY_FIND_LIBRARIES}")
        message("LIB_TRY_FIND_LINK = ${LIB_TRY_FIND_LINK}")
        message("LIB_TRY_FIND_REQUIRED = ${LIB_TRY_FIND_REQUIRED}")
        message("LIB_FILE_LOCATIONS = ${LIB_FILE_LOCATIONS}")
        message("LIB_DEPENDENCIES = ${LIB_DEPENDENCIES}")
    endif ()

endmacro()

macro(_rox_check_if_all_file_locations_exist)
    if (LIB_FILE_LOCATIONS)
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
    elseif (LIB_TRY_FIND_LIBRARIES)
        if ("${${LIB_TRY_FIND_LIBRARIES}}" STREQUAL "")
            if (LIB_VERBOSE)
                message("${LIB_NAME} lib file not found")
            endif ()
            set(LIB_FILE_NOT_FOUND 1)
        else ()
            foreach (arg IN LISTS ${LIB_TRY_FIND_LIBRARIES})
                if (NOT EXISTS ${arg})
                    if (LIB_VERBOSE)
                        message("${LIB_NAME} lib file not found: ${arg}")
                    endif ()
                    set(LIB_FILE_NOT_FOUND 1)
                endif ()
            endforeach ()
        endif ()
    elseif (LIB_TRY_FIND_TARGET_NAMES)
        foreach (arg IN LISTS LIB_TRY_FIND_TARGET_NAMES)
            if (LIB_VERBOSE)
                if (TARGET ${arg})
                    message("Target ${arg} exists")
                else ()
                    message("${LIB_NAME} target not found: ${arg}")
                endif ()
            endif ()
            if (NOT TARGET ${arg})
                set(LIB_FILE_NOT_FOUND 1)
            endif ()
        endforeach ()
    endif ()
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

            ExternalProject_Add(lib_${LIB_NAME}
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
                    PATCH_COMMAND ${LIB_PATCH}
                    BUILD_COMMAND ${LIB_BUILD}
                    BUILD_IN_SOURCE ${LIB_BUILD_IN_SOURCE}
                    DEPENDS ${LIB_DEPENDENCIES}
                    CMAKE_ARGS
                    "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")

        else ()

            ExternalProject_Add(lib_${LIB_NAME}
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
                    PATCH_COMMAND ${LIB_PATCH}
                    BUILD_COMMAND ${LIB_BUILD}
                    BUILD_IN_SOURCE ${LIB_BUILD_IN_SOURCE}
                    DEPENDS ${LIB_DEPENDENCIES}
                    CMAKE_ARGS
                    "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")

        endif ()

        if (LIB_TRY_FIND_IN_INSTALL_DIR_SET)
            if (LIB_VERBOSE)
                message("Library ${LIB_NAME} is supposed to be built locally; setting ${LIB_TRY_FIND_IN_INSTALL_DIR_SET} to ${LIB_INSTALL_DIR}")
            endif ()
            set(${LIB_TRY_FIND_IN_INSTALL_DIR_SET} ${LIB_INSTALL_DIR} PARENT_SCOPE)
        endif ()

    endif ()

endmacro()

macro(_rox_try_find_third_party_lib)

    if (LIB_TRY_FIND_IN_INSTALL_DIR)
        list(APPEND LIB_TRY_FIND_IN ${LIB_INSTALL_DIR})
    endif ()

    if (LIB_TRY_FIND_IN)
        foreach (dir IN LISTS LIB_TRY_FIND_IN)
            if (LIB_VERBOSE)
                message("Looking for ${LIB_NAME} in ${dir}")
            endif ()
            list(APPEND CMAKE_PREFIX_PATH ${dir})
        endforeach ()
    endif ()

    if (LIB_TRY_FIND)

        list(GET LIB_TRY_FIND 0 LIB_TRY_FIND_PACKAGE_NAME)

        if (NOT LIB_TRY_FIND_THEN)
            set(LIB_TRY_FIND_THEN ${LIB_TRY_FIND_PACKAGE_NAME}_FOUND)
            string(TOUPPER ${LIB_TRY_FIND_THEN} LIB_TRY_FIND_THEN)
        endif ()

        if (NOT LIB_TRY_FIND_INCLUDE_DIR)
            set(LIB_TRY_FIND_INCLUDE_DIR ${LIB_TRY_FIND_PACKAGE_NAME}_INCLUDE_DIR)
            string(TOUPPER ${LIB_TRY_FIND_INCLUDE_DIR} LIB_TRY_FIND_INCLUDE_DIR)
        endif ()

        if (LIB_TRY_FIND_REQUIRED)
            set(FIND_PACKAGE_ARGS REQUIRED)
        endif ()

        if (NOT ${LIB_TRY_FIND_THEN})
            find_package(${LIB_TRY_FIND_PACKAGE_NAME} ${FIND_PACKAGE_ARGS})
        endif ()

        if (LIB_VERBOSE)
            if (${LIB_TRY_FIND_THEN})
                message("Library ${LIB_NAME} found")
            else ()
                message("Library ${LIB_NAME} NOT found")
            endif ()
        endif ()

        if (${LIB_TRY_FIND_THEN})
            if (LIB_TRY_FIND_INCLUDE_DIR AND NOT ${LIB_TRY_FIND_INCLUDE_DIR} STREQUAL "")
                if (LIB_VERBOSE)
                    message("Including library directory ${${LIB_TRY_FIND_INCLUDE_DIR}}")
                endif ()
                include_directories(${${LIB_TRY_FIND_INCLUDE_DIR}})
            endif ()
            if (LIB_TRY_FIND_IN_INSTALL_DIR_SET AND NOT ${LIB_TRY_FIND_INCLUDE_DIR} STREQUAL "")
                string(FIND ${${LIB_TRY_FIND_INCLUDE_DIR}} ${LIB_INSTALL_DIR} SUBSTR_POS)
                if (SUBSTR_POS EQUAL 0)
                    if (LIB_VERBOSE)
                        message("Library ${LIB_NAME} found in lib install dir; setting ${LIB_TRY_FIND_IN_INSTALL_DIR_SET} to ${LIB_INSTALL_DIR}")
                    endif ()
                    set(${LIB_TRY_FIND_IN_INSTALL_DIR_SET} ${LIB_INSTALL_DIR} PARENT_SCOPE)
                else ()
                    if (LIB_VERBOSE)
                        message(WARNING "Library ${LIB_NAME} found in dir ${${LIB_TRY_FIND_INCLUDE_DIR}} which is NOT a default lib installation dir; NOT setting ${LIB_TRY_FIND_IN_INSTALL_DIR_SET}")
                    endif ()
                endif ()
            endif ()
        endif ()

        # TODO: check lib version

    endif ()
endmacro()

macro(_rox_link_third_party_lib)

    # Link with target created by find_package
    if (LIB_TRY_FIND_TARGET_NAMES)
        foreach (LIB_TRY_FIND_TARGET_NAME IN LISTS LIB_TRY_FIND_TARGET_NAMES)
            if (TARGET ${LIB_TRY_FIND_TARGET_NAME})
                if (LIB_TRY_LIB_LINK)
                    if (LIB_VERBOSE)
                        message("Linking target ${LIB_TRY_FIND_TARGET_NAME} with ${LIB_TRY_FIND_LINK}")
                    endif ()
                    set_target_properties(${LIB_TRY_FIND_TARGET_NAME} PROPERTIES IMPORTED_LINK_INTERFACE_LIBRARIES "${LIB_TRY_FIND_LINK}")
                endif ()
                if (LIB_TRY_FIND_DEFINITIONS)
                    if (LIB_VERBOSE)
                        message("Setting compile definitions for target ${LIB_TRY_FIND_TARGET_NAME} to ${LIB_TRY_FIND_DEFINITIONS}")
                    endif ()
                    set_target_properties(${LIB_TRY_FIND_TARGET_NAME} PROPERTIES INTERFACE_COMPILE_DEFINITIONS "${LIB_TRY_FIND_DEFINITIONS}")
                endif ()
                list(APPEND ROX_EXTERNAL_LIBS ${LIB_TRY_FIND_TARGET_NAME})
            elseif (LIB_VERBOSE)
                message("Target ${LIB_TRY_FIND_TARGET_NAME} NOT found. Trying to use the built one.")
            endif ()
        endforeach ()
    elseif (LIB_TRY_FIND_DEFINITIONS)
        if (LIB_VERBOSE)
            message("Setting compile definitions for target ${LIB_TRY_FIND_TARGET_NAME} to ${LIB_TRY_FIND_DEFINITIONS}")
        endif ()
        list(APPEND ROX_EXTERNAL_LIB_DEFINITIONS ${LIB_TRY_FIND_DEFINITIONS})
    endif ()

    # link with library found by find-package directly
    if (${LIB_TRY_FIND_LIBRARIES})
        list(APPEND ROX_EXTERNAL_LIBS ${${LIB_TRY_FIND_LIBRARIES}})
    endif ()

    # link with custom-build targets
    if (LIB_TARGETS)
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
                    endif ()

                    if (LIB_STATIC)
                        add_library(${LIB_TARGET_NAME} STATIC IMPORTED)
                    endif ()

                    set_target_properties(${LIB_TARGET_NAME} PROPERTIES
                            IMPORTED_LOCATION "${LIB_FILE_LOCATION}"
                            INTERFACE_INCLUDE_DIRECTORIES "${LIB_INCLUDE_DIR}")

                else ()

                    message(FATAL_ERROR "${LIB_NAME} library not found in ${LIB_FILE_LOCATION}. Build third-party libs by navigating to vendor and calling ./build-third-party-libs.sh.")

                endif ()

                if (LIB_LINK)
                    if (LIB_VERBOSE)
                        message("Linking target ${LIB_TARGET_NAME} with ${LIB_LINK}")
                    endif ()
                    set_target_properties(${LIB_TARGET_NAME} PROPERTIES IMPORTED_LINK_INTERFACE_LIBRARIES "${LIB_LINK}")
                endif ()

                if (LIB_DEFINITIONS)
                    if (LIB_VERBOSE)
                        message("Setting compile definitions for target ${LIB_TARGET_NAME} to ${LIB_DEFINITIONS}")
                    endif ()
                    target_compile_definitions(${LIB_TARGET_NAME} INTERFACE ${LIB_DEFINITIONS})
                endif ()

                list(APPEND ROX_EXTERNAL_LIBS ${LIB_TARGET_NAME})

            endif ()

        endforeach ()
    endif ()

    # TODO: copy everything from the lib's bin dir into the project dir?

endmacro()

function(rox_external_lib LIB_NAME)

    set(options VERBOSE DRY_RUN STATIC SHARED BUILD_IN_SOURCE TRY_FIND_REQUIRED TRY_FIND_IN_INSTALL_DIR)
    set(oneValueArgs VERSION URL HASH FILE CFLAGS CONFIGURE BUILD SUBDIR INCLUDE_DIR CMAKE TRY_FIND_THEN TRY_FIND_VERSION TRY_FIND_INCLUDE_DIR TRY_FIND_LIBRARIES TRY_FIND_IN_INSTALL_DIR_SET)
    set(multiValueArgs CMAKE_ARGS TARGETS DEFINITIONS PATCH LINK LINK_WIN DEPENDS_ON TRY_FIND TRY_FIND_LINK TRY_FIND_DEFINITIONS TRY_FIND_IN)

    cmake_parse_arguments(LIB "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(ROX_EXTERNAL_LIBS ${ROX_EXTERNAL_LIBS})
    set(ROX_EXTERNAL_LIB_PATHS ${ROX_EXTERNAL_LIB_PATHS})
    set(ROX_EXTERNAL_LIB_DEFINITIONS ${ROX_EXTERNAL_LIB_DEFINITIONS})
    set(ROX_EXTERNAL_LIB_RUNTIME_FILES ${ROX_EXTERNAL_LIB_RUNTIME_FILES})

    _rox_init_third_party_lib_vars()
    _rox_try_find_third_party_lib()

    if (ROX_BUILD_THIRD_PARTY_LIBS)

        if (LIB_DRY_RUN)
            return()
        endif ()

        _rox_build_third_party_lib()

    else ()

        _rox_link_third_party_lib()

        set(ROX_EXTERNAL_LIBS ${ROX_EXTERNAL_LIBS} PARENT_SCOPE)

        if (LIB_SHARED)
            link_directories(${LIB_SHARED_DIR})
            list(APPEND ROX_EXTERNAL_LIB_PATHS ${LIB_SHARED_DIR})
            file(GLOB files RELATIVE ${LIB_SHARED_DIR} "${LIB_SHARED_DIR}/*${CMAKE_SHARED_LIBRARY_SUFFIX}")
            foreach (file IN LISTS files)
                list(APPEND ROX_EXTERNAL_LIB_RUNTIME_FILES "${LIB_SHARED_DIR}/${file}")
            endforeach ()
        endif ()

        set(ROX_EXTERNAL_LIB_PATHS ${ROX_EXTERNAL_LIB_PATHS} PARENT_SCOPE)
        set(ROX_EXTERNAL_LIB_DEFINITIONS ${ROX_EXTERNAL_LIB_DEFINITIONS} PARENT_SCOPE)
        set(ROX_EXTERNAL_LIB_RUNTIME_FILES ${ROX_EXTERNAL_LIB_RUNTIME_FILES} PARENT_SCOPE)

        if (LIB_VERBOSE)
            message("ROX_EXTERNAL_LIBS = ${ROX_EXTERNAL_LIBS}")
            message("ROX_EXTERNAL_LIB_PATHS = ${ROX_EXTERNAL_LIB_PATHS}")
            message("ROX_EXTERNAL_LIB_DEFINITIONS = ${ROX_EXTERNAL_LIB_DEFINITIONS}")
            message("ROX_EXTERNAL_LIB_RUNTIME_FILES = ${ROX_EXTERNAL_LIB_RUNTIME_FILES}")
        endif ()

    endif ()

endfunction()
