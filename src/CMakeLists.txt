include_directories(.)

include(external-libs.cmake)

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
        roxx/extensions.c
        roxx/parser.c
        roxx/stack.c
        xpack/security.c
        device.c
        dynamic.c
        roxapi.c
        util.c)

add_library(libroxc ${ROX_SRC})
target_link_libraries(libroxc ${ROX_EXTERNAL_LIBS})

add_library(libroxcpp ${ROX_SRC} api.cxx)
target_link_libraries(libroxcpp ${ROX_EXTERNAL_LIBS})