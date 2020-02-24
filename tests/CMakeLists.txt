include(../src/external-libs.cmake)
include(external-libs.cmake)

enable_testing()

list(APPEND LIBS ${ROX_EXTERNAL_LIBS})
list(APPEND LIBS roxvendorlib)
list(APPEND LIBS rollout_static)

include(rox_add_tests)
include(rox_copy_shared_libs)
include_directories(. ../src)

set(ROX_TEST_SRC fixtures.c)
add_library(roxtest ${ROX_TEST_SRC})
target_link_libraries(roxtest ${LIBS})

list(APPEND LIBS roxtest)

rox_add_tests(".c" "${LIBS}")
rox_add_tests(".cpp" "${LIBS}")

set_tests_properties(test_server PROPERTIES ENVIRONMENT ROLLOUT_MODE=QA)
rox_copy_shared_libs(roxtest ${CMAKE_BINARY_DIR}/tests)