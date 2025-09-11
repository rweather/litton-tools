;
; "Hello World" example that uses a pointer to a string to fetch characters.
;
; Technically the Litton doesn't have pointers, but they are possible to
; achieve with a type of self-modifying code using the "JA" instruction.
;
    title "Hello World with Pointers"
    printer $11,"EBS1231"
    drumsize 2048
    org $300
start:
    isw printer
;
; S0 is the pointer to the string.
;
    ca message_start
    st 0
;
print_loop:
;
; Use the prototype load instruction at "load_pointer" to
; construct a full load from pointer with the address in S0.
;
    ca 0
    bls 16              ; Align the pointer at bit 16 of the word.
    ad load_pointer     ; Add the prototype instruction.
    ja                  ; Jump to the newly-created instruction.
loaded:
    tz                  ; A zero word terminates the string.
    jc done
print_byte:
    oaow                ; Output the top 8 bits of the word, with odd parity.
    tz                  ; If we have only zeroes left, then stop here.
    jc next_word
    ju print_byte
next_word:
    ca 0
    sk
    ak
    st 0
    ju print_loop

done:
    hh 0                ; Halt.

message:
    db  "Hello World with Pointers\r\n"
    db  0
message_start:
    dw  message

;
; Prototype for loading from a pointer.  This is adjusted by
; adding the pointer value to the instruction and then executed
; by jumping to the instruction with "JA".
;
    org $409            ; Align so that the top instruction becomes a no-op.
load_pointer:
    ca 0                ; Load from the adjusted pointer.
    ju loaded           ; Jump back to the main code.

    entry start
