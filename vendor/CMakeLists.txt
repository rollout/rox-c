cmake_minimum_required(VERSION 2.8)

project(rox-api-third-party-libs)

include(ExternalProject)

set(ROX_BUILD_THIRD_PARTY_LIBS 1)
set(ROX_THIRD_PARTY_LIBS_LOCATION ${CMAKE_BINARY_DIR})

include(../rox_external_lib.cmake)
include(../src/external-libs.cmake)
include(../tests/external-libs.cmake)
