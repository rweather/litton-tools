OPUS Program Tape Format
========================

OPUS saves and loads program tapes in a text format containing
addresses, word values, and punctuation.  The following is an example:

    300#645C0E0007/103FD53E11/3E943DD13C/000707E302
    380#0000000000/111FC71E11/1E961DD31C/000707E382,

Ranges of memory addresses start with a hexadecimal address,
followed by a "#", and then the hexadecimal words separated by slashes.

If there are multiple memory ranges, then each range must be terminated
by a CRLF sequence.  That informs OPUS to look for a new starting address.
The final memory range is terminated by a comma.

Spaces may appear at the start and end of the tape image as tape leaders.

All characters are encoded in the EBS1231 character set.  The emulator
converts to and from ASCII for convenience.

Programs in the high-level assembly language live between $300 and $37F,
with global variable data between $380 and $3BF.

OPUS does not restrict loaded programs to $300 to $3BF, so it is possible to
load programs and data to any location on the drum.  This can be useful when
loading a program written in the low-level assembly language.  The user
can load the program with OPUS, halt the machine, and then enter a jump
instruction on the front panel to start the low-level program.
