Example high-level OPUS programs
================================

## Loading and running the programs

Run the `litton` GUI emulator with no arguments to launch into OPUS.
Press HALT, READY, RUN in that order to start running OPUS.

Then press F5 (F1) to start loading a program tape.  The "TAPE IN"
button will highlight.  Press it and select the tape you want to load.
After loading, you can press F4 (IIII) to run the program.

## Echo

The program `echo.tape` echoes the characters typed by the user on the
keyboard to the printer.  CR is converted into CRLF and F1 (I) returns to OPUS.
Here is the pseudocode version:

    P00# SEL 11         ; Select the keyboard and printer.
         SCI            ; Single character input from keyboard into A reg.
         SCO            ; Single character output of A reg to printer.
         ACC V00        ; Add V00 to the A register.
    P01# JZP P02        ; Jump to P02 if the A register is zero.
         ACC V01        ; Add V01 to the A register.
         JZP P03        ; Jump to P03 if the A register is zero.
         JUP P00        ; Jump unconditional to P00.
    P02# CO 75          ; Print a LF after a CR character is input.
         JUP P00        ; Jump unconditional to P00.
         AJ             ; NOP padding.
         AJ
    P03# OPUS           ; Return to OPUS when the I function key is pressed.
         AJ             ; NOP padding.
         AJ
         AJ
    V00# -64            ; Negated version of the CR character code.
    V01# 37             ; Code for CR minus the code for the I function key.

The `ACC V00` instruction adds the negated version of CR to the character
in the A register.  If A contains a CR, the result will be zero and control
passes to P02 to print a LF character.

If the result is non-zero, then the `ACC V01` instruction adds the difference
between CR and the I function key to the A register.  If A originally
contained a I function key code, then the result will now be zero and
control passes to P03 to exit back to OPUS.

## Fibonacci

The program `fibonacci.tape` calculates the Fibonacci sequence and prints
it one number at a time to the printer.  Here is the pseudocode version:

    P00# BV V00         ; Set the A register to 1.
         XCB            ; Exchange A and B to put 1 into B.
         CLR            ; Clear the A register to 0.
         AJ             ; NOP padding.
    P01# ADD            ; Add B to A.
         XCB            ; Swap A and B.
         OUT V01        ; Output A using the number formatting rule in V01.
         TAB 1          ; Output a CR character.
    P02# CO 75          ; Output a LF character.
         JUP P01        ; Jump back to P01 for the next number in the sequence.
         AJ             ; NOP padding.
         AJ
    V00# 1              ; Constant set to 1.
    V01# 066666666661   ; Number formatting rule (in octal).

## HELLORLD

The program `hellorld.tape` prints the string "HELLORLD" followed by CRLF,
and then returns to OPUS.  The string is placed in V storage registers,
and printed 5 characters at a time.  Here is the psuedocode version:

    P00# BV V00         ; Bring the value of V00 into the A register.
         ALFO           ; Print the A register as 5 characters.
         BV V01         ; Bring the value of V01 into the A register.
         ALFO           ; Print the A register as 5 characters.
    P01# OPUS           ; Return to OPUS.
         AJ             ; NOP-padding of P01 to 4 instructions.
         AJ
         AJ
    V00# 241409663782   ; "HELLO"
    V01# 176684286013   ; "RLD", CR, LF

## Print Paper Tape

The program `print-tape.tape` prints the contents of a paper tape to the
printer.  Load `print-tape.tape` into the emulator and press F4 (IIII) to run.

The "TAPE IN" button will highlight.  Press the button and select the
tape file that you want to print.

Once the input tape is exhausted, the program will spin in an infinite loop
waiting for the next tape symbol.  Press HALT to stop the program.

This program was found on the drum for David Lovett's Litton computer.
It was apparently the last program the previous owner ran before the
machine was decommisioned.  Here is the pseudocode version:

    P00# SEL 21         ; Select the tape reader and the printer.
         DUP 1          ; Duplicate 1 character from tape reader to printer.
         JUP P00        ; Jump unconditional back to P00.
         AJ             ; NOP padding.
