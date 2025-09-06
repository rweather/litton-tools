
set(CORE_SOURCES 
    core/litton-device.c
    core/litton-drum.c
    core/litton-opcodes.c
    core/litton-run.c
    core/litton-state.c
)

add_executable(litton-run
    emulator/main.c
    ${CORE_SOURCES}
)
target_include_directories(litton-run PUBLIC ${CMAKE_CURRENT_LIST_DIR})
install(TARGETS litton-run DESTINATION bin)

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
