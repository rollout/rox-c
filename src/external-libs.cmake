rox_external_lib(pcre2
        # looking for preinstalled pcre2
        TRY_FIND PCRE2
        TRY_FIND_THEN PCRE2_FOUND
        TRY_FIND_INCLUDE_DIR PCRE2_INCLUDE_DIRS
        TRY_FIND_LIBRARIES PCRE2_LIBRARIES
        # otherwise build from sources
        VERSION 10.34
        URL https://ftp.pcre.org/pub/pcre/pcre2-<LIB_VERSION>.tar.gz
        HASH E3E15CCA49557A9C07A21DDE2DA05EA5
        TARGETS pcre2 pcre2-8d
        DEFINITIONS PCRE2_CODE_UNIT_WIDTH=8 PCRE2_STATIC)

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
        # looking for preinstalled openssl
        TRY_FIND OpenSSL OpenSSL::Crypto OpenSSL::SSL
        TRY_FIND_IN_INSTALL_DIR OPENSSL_ROOT_DIR
        TRY_FIND_THEN OPENSSL_FOUND
        TRY_FIND_INCLUDE_DIR OPENSSL_INCLUDE_DIR
        # otherwise build from sources
        VERSION 1.1.1
        URL https://github.com/openssl/openssl/archive/OpenSSL_1_1_1.tar.gz)

rox_external_lib(zlib
        # looking for preinstalled zlib
        TRY_FIND ZLIB ZLIB::ZLIB
        TRY_FIND_THEN ZLIB_FOUND
        TRY_FIND_INCLUDE_DIR ZLIB_INCLUDE_DIRS
        TRY_FIND_IN_INSTALL_DIR ZLIB_INSTALL_DIR
        # OR building from sources
        VERSION 1.2.11
        URL http://www.zlib.net/zlib-<LIB_VERSION>.tar.gz)

rox_external_lib(curl
        VERSION 7.68.0
        # looking for preinstalled curl
        TRY_FIND CURL CURL::libcurl
        TRY_FIND_THEN CURL_FOUND
        TRY_FIND_INCLUDE_DIR CURL_INCLUDE_DIRS
        # OR building from sources
        DEPENDS_ON openssl zlib
        URL https://github.com/curl/curl/releases/download/curl-7_68_0/curl-7.68.0.tar.gz
        CMAKE_ARGS BUILD_CURL_TESTS=OFF CURL_DISABLE_LDAP=ON BUILD_SHARED_LIBS=OFF CMAKE_USE_OPENSSL=ON OPENSSL_ROOT_DIR=${OPENSSL_ROOT_DIR} ZLIB_ROOT=${ZLIB_INSTALL_DIR}
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
else ()
    rox_external_lib(pthreads
            TRY_FIND Threads Threads::Threads
            REQUIRED)
endif ()