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
        vendor/base64.c
        vendor/md5.c
        vendor/semver.c
        vendor/strrep.c
        core.c
        device.c
        dynamic.c
        rollout.c
        server.c
        util.c
        api.cxx)

add_library(rollout_static STATIC ${ROX_SRC})
target_link_libraries(rollout_static ${ROX_EXTERNAL_LIBS})

add_library(rollout SHARED ${ROX_SRC})
target_link_libraries(rollout ${ROX_EXTERNAL_LIBS})
set_target_properties(rollout PROPERTIES PUBLIC_HEADER "../include/rollout.h")
set_target_properties(rollout PROPERTIES C_VISIBILITY_PRESET hidden)
set_target_properties(rollout PROPERTIES COMPILE_DEFINITIONS "ROX_DLL;ROX_DLL_EXPORTS")
set_target_properties(rollout PROPERTIES DEFINITIONS ROX_DLL)

INSTALL(TARGETS rollout rollout_static
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        PUBLIC_HEADER DESTINATION include)

INSTALL(FILES ${ROX_EXTERNAL_LIB_RUNTIME_FILES} DESTINATION bin)