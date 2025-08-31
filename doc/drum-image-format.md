Litton Drum Image Format
========================

This document describes a dump format for Litton magnetic drum images.
It is not based on any specific Litton format; it was designed for use
with the emulator.  It could also be used to dump actual Litton drums
or convert paper tapes into a neutral cross-platform format.

Drums usually come in sizes of either 2048 or 4096 words of memory.
Each word is 40 bits in length.  Addresses are three-digit hexadecimal
numbers like 300, FFF, A09, etc.

## Example

The following is an example from the start of the "Hello, World!"
example in this repository:

    #Litton-Drum-Image
    #Title: Hello, World!
    #Drum-Size: 2048
    #Entry-Point: 300
    #Console-Character-Set: ASCII
    #Console-Device: 11
    300:017E110A0A
    301:017848F302
    302:027865F303
    303:03786CF304
    ...

## Header

The drum file begins with 1 or more header lines starting with "#".
The file line must be "#Litton-Drum-Image".  The remaining lines
provide optional metadata about the drum image.

* `Title` - Title of the drum image; e.g. "Hello, World!", "OPUS v1", etc.
* `Drum-Size` - Size of the magnetic drum on the machine in decimal;
defaults to the maximum of 4096 if not specified.
* `Entry-Point` - Hexadecimal address of the instruction to start executing
the image at.  If the entry point is not specified, then it needs to be
provided separately to the emulator.
* `Console-Character-Set` - Expected character set for console input and
output in the program.  This is used by the emulator to mount the correct
I/O devices for the program.  Defaults to "ASCII".
* `Console-Device` - Device select code for the console device, in hexadecimal.
For example, "11" means "Group 1, Device 1".

The names of the metadata fields are case-sensitive.  Other metadata fields
may be defined in future, especially for other devices like printers and
paper tape readers/punches.

## Data

Following the header is 0 or more lines of drum word data.  There are two
hexadecimal fields separated by a colon:

* 3-digit address of the word in memory.
* 10-digit value for the 40-bit word to place at the memory address,
from most significant byte to lease significant byte.

The data continues until end of file or until another "#" line is seen.
"#" starts a new header for a new drum image.  This allows multiple
program images to be concatenated for loading into memory as a single unit.

Blank lines in the file are ignored.

## Character Sets

The following character sets are currently supported:

* ASCII - Self-explantory.
* EBS315 - Keyboard character set from section B.4 of the [EBS315 ABS 1231 Operator Manual](http://www.bitsavers.org/pdf/litton/EBS315_ABS_1231_Operator_Manual_1969.pdf).
