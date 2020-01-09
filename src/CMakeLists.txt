include_directories(.)
set(ROX_SRC stack.c parser.c api.c)
add_library(libroxc ${ROX_SRC})
add_library(libroxcpp ${ROX_SRC} api.cxx)
