Litton Minicomputer Tools
=========================

This repository contains a set of tools for emulating the behaviour
of a 1600-series Litton minicomputer, similar to that being restored by
David Lovett a.k.a Usagi Electric.

## Building

The code is designed to run under Linux systems, using SDL2 for the
graphical user interface.  You will need cmake and SDL2 installed to build it
(and obviously a C compiler like gcc):

    sudo apt install cmake libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev

Then do this from the main repository directory:

    mkdir build
    cd build
    cmake ..
    make
    sudo make install

## Running

After building and installing the tools, you can run the command-line
emulator on the examples to check basic functionality:

    litton-run examples/hello_world.drum
    litton-run examples/fibonacci.drum

Use the `-v` (verbose) option to print the register contents and disassemble
instructions as each instruction is executed:

    litton-run -v examples/fibonacci.drum

The command-line emulator will attempt to simulate the speed of the original
Litton.  Use the `-f` option (fast mode) to run at the full speed of the
host computer.

To run the GUI version of the emulator, use "litton" instead:

    litton examples/echo.drum

Once the system starts, press HALT, then READY, then RUN to start the program.
See the [EBS315 ABS 1231 Operator Manual](http://www.bitsavers.org/pdf/litton/EBS315_ABS_1231_Operator_Manual_1969.pdf)
for more information on using the computer from the front panel.

The GUI version of the emulator can load and save drum images and mount tapes.
It expects the `zenity` program to be on the PATH to handle the file dialog.
If this isn't installed already, then do the following:

    sudo apt install zenity

The machine must be halted to use the DRUM and TAPE buttons.

## Assembler

There is a cross-assembler for low-level Litton machine code in
this repository to make it easier to write new programs for the Litton.
See the [assembler documentation](doc/assembler-low-level.md)
for more information.  The programs in the `examples` directory are written
using this assembler.

## Disassembler

The `litton-disassembler` tool can disassemble images in the
[drum image file format](doc/drum-image-format.md).  The default format
tries to make the result look "pretty" by suppressing implicit jumps and
padding no-ops:

    $ litton-disassembler examples/fibonacci.drum
    700: 01 09 B0 00 10
         CL
         ST    0
         SK
    701: 02 08 21 E7 02
         AK
         XC    1
    702: 03 80 01 B0 07
         CA    1
         ST    7
    703: 04 C7 07 20 0A
         JM    $707
         XC    0
    704: 05 90 01 F7 06
         AD    1
         JC    $706
    705: 06 21 20 E7 02
         XC    1
         XC    0
         JU    $702
    ...

There is also a "raw" mode that shows the sub-instructions in columns:

    $ litton-disassembler --raw examples/fibonacci.drum
    700: 01 09 B0 00 10 | CL        | ST   0    | SK        |           | NEXT:$701
    701: 02 08 21 E7 02 | AK        | XC   1    | JU   $702 |           | NEXT:$702
    702: 03 80 01 B0 07 | CA   1    | ST   7    |           |           | NEXT:$703
    703: 04 C7 07 20 0A | JM   $707 | XC   0    | NN        |           | NEXT:$704
    704: 05 90 01 F7 06 | AD   1    | JC   $706 |           |           | NEXT:$705
    705: 06 21 20 E7 02 | XC   1    | XC   0    | JU   $702 |           | NEXT:$706
    ...

## Documentation

The following documents are from Litton and explain different aspects
of the machine:

* [EBS315 ABS 1231 Operator Manual](http://www.bitsavers.org/pdf/litton/EBS315_ABS_1231_Operator_Manual_1969.pdf)
* [Litton 1600 Technical Reference Manual](http://www.bitsavers.org/pdf/litton/Litton1600_TechnicalRefMan.pdf)

Documentation for the tools in this repository:

* [Litton Drum Image File Format](doc/drum-image-format.md)
* [Litton Low-Level Cross Assembler](doc/assembler-low-level.md)
* [Explainer for Litton Instruction Precession](doc/precession.md)

## TODO

* Input and output punch tape formats.
* Custom OPUS implementation to get a basic OS for the emulator.
* Dump the real OPUS and integrate it when possible.
* Tools for writing and disassembling program tapes in the high-level
symbolic language.

## License

Distributed under the terms of the MIT license.

## Contact

For more information on this code, to report bugs, or to suggest
improvements, please contact the author Rhys Weatherley via
[email](mailto:rhys.weatherley@gmail.com).
