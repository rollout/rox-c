rox_external_lib(pcre2 SHARED
        # looking for preinstalled pcre2
        TRY_FIND PCRE2 PCRE2::PCRE2
        TRY_FIND_IN_INSTALL_DIR
        TRY_FIND_DEFINITIONS PCRE2_CODE_UNIT_WIDTH=8
        # otherwise build from sources
        VERSION 10.34
        URL https://sourceforge.net/projects/pcre/files/pcre2/10.34/pcre2-<LIB_VERSION>.tar.gz/download
        HASH E3E15CCA49557A9C07A21DDE2DA05EA5
        CMAKE_ARGS BUILD_SHARED_LIBS=ON)

rox_external_lib(collectc STATIC
        URL https://github.com/srdja/Collections-C.git
        HASH 584e113e123ac30fe78b3e92d70f6c40a066960d
        SUBDIR src)

rox_external_lib(cjson SHARED
        # looking for preinstalled components
        TRY_FIND CJSON CJSON::CJSON
        TRY_FIND_IN_INSTALL_DIR
        # otherwise build from sources
        URL https://github.com/DaveGamble/cJSON.git
        HASH f790e17b6cecef030c4eda811149d238c2085fcf)

rox_external_lib(openssl SHARED
        # looking for preinstalled openssl
        TRY_FIND OpenSSL OpenSSL::Crypto OpenSSL::SSL
        TRY_FIND_IN_INSTALL_DIR_SET OPENSSL_ROOT_DIR
        # otherwise build from sources
        VERSION 1.1.1
        URL https://github.com/openssl/openssl/archive/OpenSSL_1_1_1.tar.gz)

rox_external_lib(zlib SHARED
        # looking for preinstalled zlib
        TRY_FIND ZLIB ZLIB::ZLIB
        TRY_FIND_IN_INSTALL_DIR_SET ZLIB_INSTALL_DIR
        # OR building from sources
        VERSION 1.2.11
        URL http://www.zlib.net/zlib-<LIB_VERSION>.tar.gz)

if (WIN32)
    # We are using win32-shim for POSIX threads here just for simplicity,
    # this allows to make identical thread API calls on all of the supported platforms.
    rox_external_lib(pthreads SHARED
            # looking for preinstalled lib
            TRY_FIND PTW32 PTW32::PTW32
            TRY_FIND_IN_INSTALL_DIR
            # OR building from sources
            URL https://github.com/martell/pthreads-win32.cmake.git
            PATCH git apply ${ROX_THIRD_PARTY_LIBS_CUSTOM_CONFIG_LOCATION}/pthreads-win32.patch
            HASH f29fd36)
else ()
    # pthreads MUST be preinstalled on POSIX systems.
    rox_external_lib(pthreads SHARED
            TRY_FIND Threads Threads::Threads
            TRY_FIND_REQUIRED)
endif ()

rox_external_lib(curl SHARED
        # looking for preinstalled curl
        TRY_FIND CURL CURL::libcurl
        TRY_FIND_LIBRARIES CURL_LIBRARIES
        TRY_FIND_IN_INSTALL_DIR
        # TODO: check curl has zlib and openssl extensions and can work with HTTPS + gzip transfer encoding
        # building from sources
        VERSION 7.68.0
        DEPENDS_ON openssl zlib pthreads
        URL https://github.com/curl/curl/releases/download/curl-7_68_0/curl-7.68.0.tar.gz
        CMAKE_ARGS BUILD_TESTING=OFF CURL_DISABLE_LDAP=ON CMAKE_USE_OPENSSL=ON OPENSSL_ROOT_DIR=${OPENSSL_ROOT_DIR} ZLIB_ROOT=${ZLIB_INSTALL_DIR})
