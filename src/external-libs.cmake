rox_external_lib(pcre2
        VERSION 10.34
        URL https://ftp.pcre.org/pub/pcre/pcre2-<LIB_VERSION>.tar.gz
        HASH E3E15CCA49557A9C07A21DDE2DA05EA5
        FILE pcre2-8d)

rox_external_lib(collectc
        URL https://github.com/srdja/Collections-C.git
        HASH 584e113e123ac30fe78b3e92d70f6c40a066960d
        SUBDIR src)

rox_external_lib(json-c
        URL https://github.com/json-c/json-c.git)
