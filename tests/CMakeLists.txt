external_lib(check 0.13.0
        https://github.com/libcheck/check/releases/download/<LIB_VERSION>/check-<LIB_VERSION>.tar.gz
        2c730c40b08482eaeb10132517970593)

enable_testing()

set(LIBS ${LIBS} libroxc check)

include_directories(. ../src)

function(add_tests)
    set(TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${ARGV0})
    file(GLOB children RELATIVE ${TEST_DIR} ${TEST_DIR}/*)
    message("Adding tests from ${TEST_DIR}")
    if (ARGV0)
        set(TEST_PATH ${ARGV0}/)
    else ()
        set(TEST_PATH "")
    endif ()
    foreach (child ${children})
        if (IS_DIRECTORY ${TEST_DIR}/${child})
            add_tests(${TEST_PATH}${child})
        else ()
            get_filename_component(TEST_FILE_EXT ${child} EXT)
            if ("${TEST_FILE_EXT}" STREQUAL ".c")
                get_filename_component(TEST_NAME ${child} NAME_WE)
                add_executable(${TEST_NAME} ${TEST_PATH}${child})
                add_test(${TEST_NAME} ${CMAKE_CURRENT_BINARY_DIR}/${TEST_PATH}${TEST_NAME})
                target_link_libraries(${TEST_NAME} ${LIBS})
            endif ()
        endif ()
    endforeach ()
endfunction()

if (NOT ROX_BUILD_THIRD_PARTY_LIBS)
    add_tests()
endif ()