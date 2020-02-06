if (WIN32)

    find_package(Perl)
    if (NOT PERL_FOUND)
        message(FATAL_ERROR "Cannot build test suites without Perl")
    endif ()

    # TODO: check if NASM is in Path

    if (CMAKE_CL_64)
        set(OPENSSL_VERSION VC-WIN64A)
    else ()
        set(OPENSSL_VERSION VC-WIN32)
    endif ()

    set(OPENSSL_CONFIGURE_COMMAND perl Configure ${OPENSSL_VERSION})
    set(LIB_LINK crypt32 wsock32 ws2_32)

else ()
    set(OPENSSL_CONFIGURE_COMMAND ./config)
endif ()

set(LIB_BUILD_IN_SOURCE 1)
set(LIB_CONFIGURE ${OPENSSL_CONFIGURE_COMMAND} --prefix=${LIB_INSTALL_DIR} --openssldir=${LIB_INSTALL_DIR}/store)
