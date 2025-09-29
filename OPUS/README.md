
This directory contains a dump of the whole drum from David Lovett's Litton,
and a cut-down version that just contains the Operating Utility System, OPUS
without any user applications or data.

The file `opus-annotated.txt` is an attempt to disassemble all of OPUS
to aid in understanding.  This is still a work in progress.

Some commands are working with the emulator and some are not or haven't
been tested yet.  This list will be updated as more commands are made to work.

* P1 - Read Program Tape - tries to do something, not fully verified
* P2 - Verify Program Tape - tries to do something, not fully verified
* IIII - Run Application - tries to do something, not fully verified
* H - Select Hexadecimal Format - working
* L - Punch Tape Leader - writes "EMPTY ERR" and stops, partially working
* N - Select Decimal Format - working
* O - Select Octal Format - working
* P - Select Program Registers - working
* Q - Change Register - crashing
* R - Reset Memory - working
* S - Store Instructions or Data - crashing
* T - Punch Program onto Tape - crashing
* V - Select Store Registers - working
* W - Print Registers - crashing
* X - Print Register A - crashing
* Y - Print Register B - crashing

The following undocumented commands are also present in the command list:

* J - ???
* K - ???
* G, I, M, U, Z - Same as P - working
