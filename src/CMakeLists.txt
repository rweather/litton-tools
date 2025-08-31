
add_executable(litton-1600
    main.c
    core/litton-device.c
    core/litton-run.c
    core/litton-state.c
)
target_include_directories(litton-1600 PUBLIC ${CMAKE_CURRENT_LIST_DIR})

install(TARGETS litton-1600 DESTINATION bin)
