rox_external_lib(pcre2
        VERSION 10.34
        URL https://ftp.pcre.org/pub/pcre/pcre2-<LIB_VERSION>.tar.gz
        HASH E3E15CCA49557A9C07A21DDE2DA05EA5
        FILE pcre2-8d)

rox_external_lib(collectc
        URL https://github.com/srdja/Collections-C.git
        HASH 584e113e123ac30fe78b3e92d70f6c40a066960d
        SUBDIR src)

rox_external_lib(cjson
        URL https://github.com/DaveGamble/cJSON.git
        HASH f790e17b6cecef030c4eda811149d238c2085fcf
        CMAKE_ARGS BUILD_SHARED_AND_STATIC_LIBS=On
        FILE libcjson)

rox_external_lib(openssl
        VERSION 1.1.1
        # looking for preinstalled openssl
        TRY_FIND OpenSSL OpenSSL::Crypto OpenSSL::SSL
        TRY_FIND_IN_INSTALL_DIR OPENSSL_ROOT_DIR
        TRY_FIND_THEN OPENSSL_FOUND
        TRY_FIND_INCLUDE_DIR OPENSSL_INCLUDE_DIR
        # OR building from sources
        URL https://github.com/openssl/openssl/archive/OpenSSL_1_1_1.tar.gz)

rox_external_lib(curl
        VERSION 7.68.0
        # looking for preinstalled curl
        TRY_FIND CURL CURL::libcurl
        TRY_FIND_THEN CURL_FOUND
        TRY_FIND_INCLUDE_DIR CURL_INCLUDE_DIRS
        # OR building from sources
        DEPENDS_ON openssl
        URL https://github.com/curl/curl/releases/download/curl-7_68_0/curl-7.68.0.tar.gz
        CMAKE_ARGS BUILD_CURL_TESTS=OFF CURL_DISABLE_LDAP=ON BUILD_SHARED_LIBS=OFF CMAKE_USE_OPENSSL=ON OPENSSL_ROOT_DIR=${OPENSSL_ROOT_DIR}
        TARGETS CURL::libcurl libcurl-d
        DEFINITIONS CURL_STATICLIB
        LINK_WIN crypt32 wsock32 ws2_32)

if (WIN32)
    rox_external_lib(pthreads-win32
            URL https://github.com/martell/pthreads-win32.cmake.git
            CMAKE_ARGS PTHREADS_BUILD_STATIC=ON
            PATCH git apply ${ROX_THIRD_PARTY_LIBS_CUSTOM_CONFIG_LOCATION}/pthreads-win32.patch
            HASH f29fd36
            FILE pthreadsVC2d
            DEFINITIONS PTW32_STATIC_LIB)
endif ()