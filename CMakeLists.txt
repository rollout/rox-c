cmake_minimum_required(VERSION 2.8)

project(rox-api)

enable_testing()

set(ROX_THIRD_PARTY_LIBS_LOCATION ${CMAKE_SOURCE_DIR}/vendor/build)
include(rox_external_lib.cmake)

include_directories(include)

add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(tests)
