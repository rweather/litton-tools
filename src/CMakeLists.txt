
set(CORE_SOURCES 
    core/litton-device.c
    core/litton-drum.c
    core/litton-front-panel.c
    core/litton-hl-opcodes.c
    core/litton-opcodes.c
    core/litton-run.c
    core/litton-state.c
    core/litton-opus.h
)

add_executable(litton-run
    emulator/main.c
    ${CORE_SOURCES}
)
target_include_directories(litton-run PUBLIC ${CMAKE_CURRENT_LIST_DIR})
install(TARGETS litton-run DESTINATION bin)

add_executable(litton
    emulator-sdl/main.c
    emulator-sdl/img-background.c
    emulator-sdl/img-buttons.c
    emulator-sdl/img-control-up.c
    emulator-sdl/img-control-down.c
    emulator-sdl/img-knob-A0.c
    emulator-sdl/img-knob-A8.c
    emulator-sdl/img-knob-A16.c
    emulator-sdl/img-knob-A24.c
    emulator-sdl/img-knob-A32.c
    emulator-sdl/img-knob-I0.c
    emulator-sdl/img-knob-I8.c
    emulator-sdl/img-knob-I16.c
    emulator-sdl/img-knob-I24.c
    emulator-sdl/img-knob-I32.c
    emulator-sdl/img-lamps.c
    emulator-sdl/font-dotmatrix.c
    ${CORE_SOURCES}
)
target_include_directories(litton PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(litton
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    ${SDL2_TTF_LIBRARIES}
    m
)
install(TARGETS litton DESTINATION bin)

add_executable(litton-as
    assembler/main.c
    assembler/assem.c
    assembler/assem.h
    assembler/drum-image.c
    assembler/drum-image.h
    assembler/symbol.c
    assembler/symbol.h
    assembler/tokeniser.c
    assembler/tokeniser.h
    ${CORE_SOURCES}
)
target_include_directories(litton-as PUBLIC ${CMAKE_CURRENT_LIST_DIR})
install(TARGETS litton-as DESTINATION bin)

add_executable(litton-disassembler
    disassembler/main.c
    ${CORE_SOURCES}
)
target_include_directories(litton-disassembler PUBLIC ${CMAKE_CURRENT_LIST_DIR})
install(TARGETS litton-disassembler DESTINATION bin)
