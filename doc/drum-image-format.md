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
    #Printer-Character-Set: EBS1231
    #Printer-Device: 41
    300:007C41F301
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
* `Keyboard-Character-Set` - Expected character set for keyboard input.
This is used by the emulator to mount the correct keyboard device for the
program.  Defaults to "EBS1231".
* `Keyboard-Device` - Device select code for the keyboard device,
in hexadecimal.
* `Printer-Character-Set` - Expected character set for printer output
in the program.  This is used by the emulator to mount the correct
printer device for the program.  Defaults to "EBS1231".
* `Printer-Device` - Device select code for the printer device, in hexadecimal.

The names of the metadata fields are case-sensitive.  Other metadata fields
may be defined in future, especially for other devices like paper tape
readers/punches.

## Data

Following the header is 0 or more lines of drum word data.  There are two
hexadecimal fields separated by a colon:

* 3-digit address of the word in memory.
* 10-digit value for the 40-bit word to place at the memory address,
from most significant byte to least significant byte.

The data continues until end of file.

Blank lines in the file are ignored.

## Character Sets

The following character sets are currently supported:

* EBS1231 - [Character set](character-set.md) from Appendix V of the
EBS/1231 System Programming Manual.  This is the native character
set of the Litton.
* ASCII - Self-explantory.
* UASCII - Uppercase-only ASCII.
