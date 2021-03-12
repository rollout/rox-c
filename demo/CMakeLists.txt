cmake_minimum_required(VERSION 3.9)

project(rollout-demo)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

find_package(ROX REQUIRED)

set(ROX_DEMOS basic logging freeze overrides)

foreach (demo ${ROX_DEMOS})
    add_executable(rollout_${demo}_demo ${demo}.c)
    target_link_libraries(rollout_${demo}_demo ROX::SDK)
    add_executable(rollout_${demo}_demo_cpp ${demo}.cpp)
    target_link_libraries(rollout_${demo}_demo_cpp ROX::SDK)
endforeach ()
