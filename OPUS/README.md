
This directory contains a dump of the whole drum from David Lovett's Litton,
and a cut-down version that just contains the Operating Utility System, OPUS
without any user applications or data.

The file `opus-annotated.txt` is an attempt to disassemble all of OPUS
to aid in understanding.  This is still a work in progress.

Some commands are working with the emulator and some are not or haven't
been tested yet.  This list will be updated as more commands are made to work.

* P1 - Read Program Tape - working
* P2 - Verify Program Tape - working
* IIII - Run Application - working
* H - Select Hexadecimal Format - working
* L - Punch Tape Leader - working
* N - Select Decimal Format - working
* O - Select Octal Format - working
* P - Select Program Registers - working
* Q - Change Register - working
* R - Reset Memory - working
* S - Store Instructions or Data - working
* T - Punch Program onto Tape - working
* V - Select Store Registers - working
* W - Print Registers - working
* X - Print Register A - working
* Y - Print Register B - working

The following undocumented commands are also present in the command list:

* J - ???
* K - ???
* G, I, M, U, Z - Same as P - working

The last high-level program that was run on the machine is a routine that
prints the contents of a tape to the printer:

    000: SEL 21                ; Select tape reader and printer.
         DUP 1                 ; Read from tape and write to the printer.
         JUP 000               ; Jump to 000.
         AJ                    ; NOP padding.
