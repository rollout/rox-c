cmake_minimum_required(VERSION 3.9)

project(rox-api-third-party-libs)

include(ExternalProject)

set(ROX_BUILD_THIRD_PARTY_LIBS 1)
set(ROX_THIRD_PARTY_LIBS_LOCATION ${CMAKE_BINARY_DIR})
set(ROX_THIRD_PARTY_LIBS_CUSTOM_CONFIG_LOCATION ${PROJECT_SOURCE_DIR}/cmake)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/../cmake/modules)
set(CMAKE_BUILD_TYPE Release)

include(rox_external_lib)

include(../src/external-libs.cmake)
include(../tests/external-libs.cmake)
