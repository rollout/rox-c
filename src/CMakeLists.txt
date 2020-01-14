include_directories(.)

set(ROX_SRC
        util.c
        core/entities.c
        roxx/stack.c
        roxx/parser.c
        api.c)

add_library(libroxc ${ROX_SRC})
add_library(libroxcpp ${ROX_SRC} api.cxx)
