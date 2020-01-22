include(../src/external-libs.cmake)
include(external-libs.cmake)

enable_testing()

set(LIBS ${LIBS} pcre2 collectc check cjson roxvendorlib)
set(LIBS_C ${LIBS} libroxc)
set(LIBS_CXX ${LIBS} libroxcpp)

include(rox_add_tests)
include_directories(. ../src)

rox_add_tests(".c" "${LIBS_C}")
rox_add_tests(".cpp" "${LIBS_CXX}")