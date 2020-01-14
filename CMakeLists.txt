cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeModules/")

project(rox-api)

enable_testing()

include(ExternalProject)
include(thirdparty/pcre.cmake)

include_directories(include)

add_subdirectory(include)
add_subdirectory(thirdparty)
add_subdirectory(src)
add_subdirectory(tests)