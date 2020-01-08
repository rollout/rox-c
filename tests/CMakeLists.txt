enable_testing()

find_package(Check REQUIRED)
set(CHECK_INCLUDE_DIRS ${Check_DIR}/../../../include)
set(CHECK_LIBRARIES ${Check_DIR}/../../check.lib)

include_directories(${Check_DIR}/../../../include)
set(LIBS ${LIBS} ${CHECK_LIBRARIES} libroxc)
set(LIBSCXX ${LIBS} ${CHECK_LIBRARIES} libroxcpp)

include_directories(. ../src)
add_executable(test_roxparser test_roxparser.c)
add_test(test_roxparser ${CMAKE_CURRENT_BINARY_DIR}/test_roxparser)
target_link_libraries(test_roxparser ${LIBS})