include(../src/external-libs.cmake)
include(external-libs.cmake)

enable_testing()

set(LIBS ${ROX_EXTERNAL_LIBS} roxvendorlib)
set(LIBS_C ${LIBS} libroxc)
set(LIBS_CXX ${LIBS} libroxcpp)

include(rox_add_tests)
include_directories(. ../src)

set(ROX_TEST_SRC fixtures.c)
add_library(libroxtest ${ROX_TEST_SRC})
target_link_libraries(libroxtest ${LIBS})

set(LIBS_C ${LIBS_C} libroxtest)
set(LIBS_CXX ${LIBS_CXX} libroxtest)

rox_add_tests(".c" "${LIBS_C}")
rox_add_tests(".cpp" "${LIBS_CXX}")

set_tests_properties(test_server PROPERTIES ENVIRONMENT ROLLOUT_MODE=QA)