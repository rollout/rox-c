cmake_minimum_required(VERSION 3.9)

project(rollout-sdk)

set(CMAKE_CXX_STANDARD 11)

set(ROX_VERSION_MAJOR 1)
set(ROX_VERSION_MINOR 0)
set(ROX_VERSION_PATCH 0)
set(ROX_VERSION_STRING ${ROX_VERSION_MAJOR}.${ROX_VERSION_MINOR}.${ROX_VERSION_PATCH})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake/modules)
set(ROX_THIRD_PARTY_LIBS_LOCATION ${CMAKE_SOURCE_DIR}/vendor/build)
set(ROX_THIRD_PARTY_LIBS_CUSTOM_CONFIG_LOCATION ${CMAKE_SOURCE_DIR}/vendor/cmake)

include(rox_external_lib)

include_directories(include)
include_directories(${CMAKE_SOURCE_DIR}/vendor/include)
link_directories(${CMAKE_SOURCE_DIR}/vendor/build/src)

add_subdirectory(include)
add_subdirectory(src)

if (NOT ROLLOUT_SKIP_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif ()

if (ROLLOUT_FIND_LEAKS)
    include(CTest)
    find_program(MEMORYCHECK_COMMAND valgrind)
    set(MEMORYCHECK_COMMAND_OPTIONS "--trace-children=yes --leak-check=full")
    set(MEMORYCHECK_SUPPRESSIONS_FILE "${PROJECT_SOURCE_DIR}/valgrind_suppress.txt")
endif ()
