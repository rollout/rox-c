cmake_minimum_required(VERSION 2.8)

project(rox-api)

enable_testing()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake/modules)
set(ROX_THIRD_PARTY_LIBS_LOCATION ${CMAKE_SOURCE_DIR}/vendor/build)

include(rox_external_lib)

include_directories(include)
include_directories(${CMAKE_SOURCE_DIR}/vendor/include)
link_directories(${CMAKE_SOURCE_DIR}/vendor/build/src)

add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(tests)
