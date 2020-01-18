enable_testing()

include_directories(. ../src)
include(rox_add_tests)

rox_add_tests(".c" roxvendorlib)