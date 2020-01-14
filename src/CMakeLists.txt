include_directories(.)

external_lib(pcre2 10.34
        https://ftp.pcre.org/pub/pcre/pcre2-<LIB_VERSION>.tar.gz
        E3E15CCA49557A9C07A21DDE2DA05EA5
        pcre2-8d.lib)

if (NOT ROX_BUILD_THIRD_PARTY_LIBS)

    set(ROX_SRC
            util.c
            core/entities.c
            roxx/stack.c
            roxx/parser.c
            roxapi.c)

    add_library(libroxc ${ROX_SRC})
    target_link_libraries(libroxc pcre2)

    add_library(libroxcpp ${ROX_SRC} api.cxx)
    target_link_libraries(libroxcpp pcre2)

endif ()