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
        URL https://github.com/openssl/openssl/archive/OpenSSL_1_1_1.tar.gz
        TRY_FIND OpenSSL
        TRY_FIND_THEN OPENSSL_FOUND
        TRY_FIND_IN_INSTALL_DIR OPENSSL_ROOT_DIR
        TRY_FIND_INCLUDE_DIR OPENSSL_INCLUDE_DIR
        TARGETS OpenSSL::Crypto libcrypto)

rox_external_lib(curl
        VERSION 7.68.0
        URL https://github.com/curl/curl/releases/download/curl-7_68_0/curl-7.68.0.tar.gz
        FILE libcurl-d_imp
        LINK libcurl-d
        DEPENDS_ON openssl)