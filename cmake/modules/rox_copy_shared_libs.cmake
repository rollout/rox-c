function(rox_copy_shared_libs TARGET DEST)
    foreach (file IN LISTS ROX_EXTERNAL_LIB_RUNTIME_FILES)
        add_custom_command(
                TARGET ${TARGET} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different ${file} ${DEST}
                COMMENT "Copying shared lib ${file} to ${DEST}")
    endforeach ()
endfunction()