include_directories(.)

include(external-libs.cmake)

set(LIBS ${LIBS} pcre2)

set(ROX_SRC
        util.c
        core/entities.c
        roxx/stack.c
        roxx/parser.c
        roxapi.c)

add_library(libroxc ${ROX_SRC})
target_link_libraries(libroxc ${LIBS})

add_library(libroxcpp ${ROX_SRC} api.cxx)
target_link_libraries(libroxcpp ${LIBS})