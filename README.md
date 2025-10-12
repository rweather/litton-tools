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

    litton-run examples/low-level/hello_world.drum
    litton-run examples/low-level/fibonacci.drum

Use the `-v` (verbose) option to print the register contents and disassemble
instructions as each instruction is executed:

    litton-run -v examples/low-level/fibonacci.drum

The command-line emulator will attempt to simulate the speed of the original
Litton.  Use the `-f` option (fast mode) to run at the full speed of the
host computer.

To run the GUI version of the emulator, use "litton" instead:

    litton

This will launch the emulator using a built-in copy of OPUS based on the
file `OPUS/opus.drum`.  You can instead provide a custom drum image on the
command-line:

    litton OPUS/whole.drum
    litton examples/low-level/echo.drum

Once the system starts, press HALT, then READY, then RUN to start the program.
See the [EBS315 ABS 1231 Operator Manual](manuals/EBS315_ABS_1231_Operator_Manual_1969.pdf)
for more information on using the computer from the front panel.

The GUI version of the emulator can load and save drum images and mount tapes.
It expects the `zenity` program to be on the PATH to handle the file dialog.
If this isn't installed already, then do the following:

    sudo apt install zenity

The machine must be halted to use the DRUM load and save buttons.

When running OPUS, the following keys do useful things:

* F1 - "I" function key - Terminate a line of input in "S" / store mode.
* F4 - "IIII" function key - Run the high-level application program in memory.
* F5 - "P1" function key - Load a program from paper tape.
* T - Save the high-level application program in memory to paper tape.

The other Litton function keys are mapped as follows:

* F2 - "II" function key.
* F3 - "III" function key.
* F6 - "P2" function key.
* F7 - "P3" function key.
* F8 - "P4" function key.

When loading from or saving to paper tape, the TAPE IN or TAPE OUT button
will highlight.  Press the highlighted button to select a tape file.
To close a tape file, press the button and then immediately cancel the
file dialog.

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

    $ litton-disassembler examples/low-level/fibonacci.drum
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

    $ litton-disassembler --raw examples/low-level/fibonacci.drum
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

* [EBS315 ABS 1231 Operator Manual](manuals/EBS315_ABS_1231_Operator_Manual_1969.pdf)
* [Litton 1600 Technical Reference Manual](manuals/Litton1600_TechnicalRefMan.pdf)
* [EBS/1231 System Programming Manual](manuals/Litton_1231_Programming_Manual.pdf)

Documentation for the tools in this repository:

* [Litton Drum Image File Format](doc/drum-image-format.md)
* [Litton Low-Level Cross Assembler](doc/assembler-low-level.md)
* [Litton Device Identifiers](doc/devices.md)
* [Explainer for Litton instruction precession](doc/precession.md)
* [Keyboard Mapping for the GUI Emulator](images/keyboard-layout.png)
* [Litton EBS1231 Character set](doc/character-set.md)
* [Usage of tracks on the drum](doc/tracks.md)
* [OPUS program tape format](doc/tape-format.md)
* [Mapping of Litton part numbers to 74xx series part numbers](doc/Litton-Part-Number-Mapping.ods) (ODS spreadsheet)
* [Location of Mxxx part designators on the PCB's and in the schematic](doc/Litton-Part-Locator.ods) (ODS spreadsheet)

## TODO

* Input and output punch tape formats.
* Dump OPUS of the real Litton and integrate it into the emulator.
* Reverse-engineer OPUS and produce an annotated assembly listing.
* Figure out the high-level symbolic language that is provided by OPUS.
* Tools for writing and disassembling program tapes in the high-level
symbolic language.

## License

Distributed under the terms of the MIT license.

## Contact

For more information on this code, to report bugs, or to suggest
improvements, please contact the author Rhys Weatherley via
[email](mailto:rhys.weatherley@gmail.com).
