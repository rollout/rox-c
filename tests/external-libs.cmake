rox_external_lib(check SHARED
        # looking for preinstalled components
        TRY_FIND Check Check::Check
        TRY_FIND_IN_INSTALL_DIR
        # otherwise build from sources
        VERSION 0.13.0
        URL https://github.com/libcheck/check/releases/download/<LIB_VERSION>/check-<LIB_VERSION>.tar.gz
        HASH 2c730c40b08482eaeb10132517970593)

rox_external_lib(catch2 SHARED
        VERSION 2.11.1
        URL https://github.com/catchorg/Catch2/archive/v<LIB_VERSION>.tar.gz
        HASH 4b097ffafd5d1102e961a14699f5b918
        CMAKE_ARGS BUILD_TESTING=OFF CATCH_INSTALL_DOCS=OFF
        HEADERS_ONLY)
