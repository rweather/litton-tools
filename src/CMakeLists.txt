
add_executable(litton-run
    emulator/main.c
    core/litton-device.c
    core/litton-drum.c
    core/litton-opcodes.c
    core/litton-run.c
    core/litton-state.c
)
target_include_directories(litton-run PUBLIC ${CMAKE_CURRENT_LIST_DIR})

install(TARGETS litton-run DESTINATION bin)
