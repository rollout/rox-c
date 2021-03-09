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
        core/security.c
        eval/extensions.c
        eval/parser.c
        eval/stack.c
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
        collections.c
        core.c
        device.c
        values.c
        util.c
        server.c
        server.cpp)

set(ROX_COMPILE_DEFINITIONS
        ROX_LIB_VERSION="${ROX_VERSION_STRING}")

if (ROX_CLIENT)
    list(APPEND ROX_SRC
            freeze.c
            overrides.c
            storage.c
            client.c
            client.cpp)
endif ()

if (ROX_CLIENT)
    list(APPEND ROX_COMPILE_DEFINITIONS ROX_CLIENT)
endif ()

if (APPLE)
    find_library(CoreFoundation CoreFoundation)
    find_library(IOKit IOKit)
    list(APPEND ROX_EXTERNAL_LIBS ${CoreFoundation})
    list(APPEND ROX_EXTERNAL_LIBS ${IOKit})
endif ()

add_library(rollout_static STATIC ${ROX_SRC})
target_link_libraries(rollout_static ${ROX_EXTERNAL_LIBS})

add_library(rollout SHARED ${ROX_SRC})
target_link_libraries(rollout ${ROX_EXTERNAL_LIBS})

set_target_properties(rollout PROPERTIES
        C_VISIBILITY_PRESET hidden
        COMPILE_DEFINITIONS "${ROX_COMPILE_DEFINITIONS};ROX_DLL;ROX_DLL_EXPORTS"
        VERSION ${ROX_VERSION_STRING}
        SOVERSION ${ROX_VERSION_MAJOR})

set_target_properties(rollout_static PROPERTIES
        COMPILE_DEFINITIONS "${ROX_COMPILE_DEFINITIONS}")

INSTALL(TARGETS rollout rollout_static
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

if (WIN32)
    set(ROX_RUNTIME_DIR bin)
else ()
    set(ROX_RUNTIME_DIR lib)
endif ()

INSTALL(FILES ${ROX_EXTERNAL_LIB_RUNTIME_FILES} DESTINATION ${ROX_RUNTIME_DIR})

if (ROX_CLIENT)

    INSTALL(DIRECTORY "../include/rox" DESTINATION include)
    INSTALL(DIRECTORY "../include/roxx" DESTINATION include)

else ()

    INSTALL(DIRECTORY "../include/rox" DESTINATION include
            PATTERN "client.h" EXCLUDE
            PATTERN "freeze.h" EXCLUDE
            PATTERN "overrides.h" EXCLUDE
            PATTERN "storage.h" EXCLUDE)

    INSTALL(DIRECTORY "../include/roxx" DESTINATION include
            PATTERN "client.h" EXCLUDE
            PATTERN "freeze.h" EXCLUDE
            PATTERN "overrides.h" EXCLUDE
            PATTERN "storage.h" EXCLUDE)

endif ()
