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
        core/logging.c
        core/network.c
        core/properties.c
        core/reporting.c
        core/repositories.c
        roxx/extensions.c
        roxx/parser.c
        roxx/stack.c
        xpack/analytics/client.c
        xpack/analytics/model.c
        xpack/configuration.c
        xpack/impression.c
        xpack/network.c
        xpack/notifications.c
        xpack/reporting.c
        xpack/security.c
        core.c
        device.c
        dynamic.c
        roxapi.c
        server.c
        util.c)

add_library(libroxc ${ROX_SRC})
target_link_libraries(libroxc ${ROX_EXTERNAL_LIBS})
target_compile_definitions(libroxc PUBLIC PCRE2_CODE_UNIT_WIDTH=8 PCRE2_STATIC)

add_library(libroxcpp ${ROX_SRC} api.cxx)
target_link_libraries(libroxcpp ${ROX_EXTERNAL_LIBS})
target_compile_definitions(libroxcpp PUBLIC PCRE2_CODE_UNIT_WIDTH=8 PCRE2_STATIC)
