OPUS
====

This directory contains a dump of the whole drum from David Lovett's Litton,
and a cut-down version that just contains the Operating Utility System, OPUS
without any user applications or data.

The file `opus-annotated.txt` is an attempt to disassemble all of OPUS
to aid in understanding.  This is still a work in progress.

## Commands

All of these OPUS commands have been tested and are working.  See the
[EBS/1231 System Programming Manual](../manuals/Litton_1231_Programming_Manual.pdf)
for more information on how to use these commands.

* P1 - Read Program Tape
* P2 - Verify Program Tape
* IIII - Run Application
* H - Select Hexadecimal Format
* L - Punch Tape Leader
* N - Select Decimal Format
* O - Select Octal Format
* P - Select Program Registers
* Q - Change Register
* R - Reset Memory
* S - Store Instructions or Data
* T - Punch Program onto Tape
* V - Select Store Registers
* W - Print Registers
* X - Print Register A
* Y - Print Register B

The following undocumented commands perform the same operation as P:

* G, I, M, U, Z - Select Program Registers

## Working with Native Code

Native programs can be loaded from paper tape using "P1".  The region
between $800 and $EFF can be used to load native code.  This region
overlaps with the distribution registers, so be careful mixing native
code with high-level assembly code.

The following undocumented commands are specific to working with
native code:

* J - Jump to Native Code
* K - Print Native Accumulator

To use the "J" command, first press "H" to go into hexadecimal mode,
press "J", type in the hexadecimal address, and then press "#" to
start running the code:

    H
    J   800#

The "K" command prints the contents of the native accumulator that is
passed to a native program with "J" and returned by that native program
when it returns to OPUS.  Press "H" to go into hexadecimal mode and
then press "K" to print the accumulator's contents.

If you want to set the native accumulator value before using "J",
then set the value at address $4EA in memory:

    H
    S  4EA#           AAAA555577
    S  4EB#                         ; Press Enter to stop.
    K  001#AA AA55 5577

"S" in hexadecimal mode can be used to set any writable location in memory.
The "W" command can be used to dump memory ranges in hexadecimal:

    H
    W   700#703#
    700#              1B 21 4000 21
    701#              01 783B F6FC
    702#              00 00 00 00 10
    703#              8F F571 4000

The "W" command groups 8-bit and 16-bit instructions to make it easier to
read blocks of native machine code instructions.

The "T" command can be used to save native programs to paper tape,
when combined with hexadecimal mode.  For example, the following will
save the words between $800 and $8FF in memory to paper tape:

    H
    T   800#8FF#

## Last Program on the Drum

The last high-level program that was run on the machine is a routine that
prints the contents of a tape to the printer:

    000: SEL 21                ; Select tape reader and printer.
         DUP 1                 ; Read from tape and write to the printer.
         JUP 000               ; Jump to 000.
         AJ                    ; NOP padding.
