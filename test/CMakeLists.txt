
enable_testing()

# Function to assemble a Litton program and arrange to run it as a test case.
function(litton_test name)
    add_custom_command(
        OUTPUT ${name}.drum
        COMMAND litton-as -o ${name}.drum ${CMAKE_CURRENT_LIST_DIR}/${name}.las
        DEPENDS ${CMAKE_CURRENT_LIST_DIR}/${name}.las
    )
    add_custom_target(${name} ALL DEPENDS ${name}.drum)
    add_test(
        NAME ${name}
        COMMAND litton-run -f ${name}.drum
    )
endfunction()

# Test cases.
litton_test(add)
