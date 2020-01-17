cmake_minimum_required(VERSION 2.8)

project(rox-api-third-party-libs)

include(ExternalProject)

set(ROX_BUILD_THIRD_PARTY_LIBS 1)
set(ROX_THIRD_PARTY_LIBS_LOCATION ${CMAKE_BINARY_DIR})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/../cmake/modules)

include(rox_add_tests)
include(rox_external_lib)

include(../src/external-libs.cmake)
include(../tests/external-libs.cmake)
