enable_testing()

find_package(Check REQUIRED)
set(CHECK_INCLUDE_DIRS ${Check_DIR}/../../../include)
set(CHECK_LIBRARIES ${Check_DIR}/../../check.lib)

include_directories(${CHECK_INCLUDE_DIRS})
set(LIBS ${LIBS} ${CHECK_LIBRARIES} libroxc)

include_directories(. ../src)

add_executable(test_stack roxx/test_stack.c)
add_test(test_stack ${CMAKE_CURRENT_BINARY_DIR}/test_stack)
target_link_libraries(test_stack ${LIBS})

add_executable(test_parser roxx/test_parser.c)
add_test(test_parser ${CMAKE_CURRENT_BINARY_DIR}/test_parser)
target_link_libraries(test_parser ${LIBS})

if (CMAKE_CONFIGURATION_TYPES)
    add_custom_target(check COMMAND
            ${CMAKE_CTEST_COMMAND}
            --force-new-ctest-process
            --output-on-failure
            --build-config "$<CONFIGURATION>")
else ()
    add_custom_target(check COMMAND
            ${CMAKE_CTEST_COMMAND}
            --force-new-ctest-process
            --output-on-failure)
endif ()