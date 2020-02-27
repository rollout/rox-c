cmake_minimum_required(VERSION 3.9)

project(rollout-demo)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

find_package(Rollout REQUIRED)

add_executable(rollout_basic_demo basic.c)
target_link_libraries(rollout_basic_demo ROLLOUT::SDK)
