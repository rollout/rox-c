include_directories(.)

include(external-libs.cmake)

set(LIBS ${LIBS} pcre2 collectc cjson)

set(ROX_SRC
        core/configuration/models.c
        core/impression/models.c
        core/client.c
        core/configuration.c
        core/consts.c
        core/context.c
        core/entities.c
        core/impression.c
        core/network.c
        core/properties.c
        core/reporting.c
        core/repositories.c
        core/security.c
        roxx/extensions.c
        roxx/parser.c
        roxx/stack.c
        roxapi.c
        util.c)

add_library(libroxc ${ROX_SRC})
target_link_libraries(libroxc ${LIBS})

add_library(libroxcpp ${ROX_SRC} api.cxx)
target_link_libraries(libroxcpp ${LIBS})