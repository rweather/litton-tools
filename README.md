Litton Minicomputer Tools
=========================

This repository contains a set of tools for emulating the behaviour
of a 1600-series Litton minicomputer, similar to that being restored by
David Lovett a.k.a Usagi Electric.

## Building

The code is designed to run under Linux systems.  You will need
cmake and gcc installed to build it.

    mkdir build
    cd build
    cmake ..
    make
    sudo make install

## Running

After building and installing the tools, you can run the emulator
on the examples:

    litton-run examples/hello_world.drum
    litton-run examples/fibonacci.drum

Use the `-v` (verbose) option to disassemble instructions as they are executed:

    litton-run -v examples/fibonacci.drum

## Assembler

There is a cross-assembler for low-level Litton machine code in
this repository to make it easier to write new programs for the Litton.
See the [assembler documentation](doc/assembler-low-level.md)
for more information.  The programs in the `examples` directory are written
using this assembler.

## Disassembler

The `litton-disassembler` tool can disassemble images in the
[drum image file format](doc/drum-image-format.md).  By default the
disassembler uses a "pretty" format that tries to show the code as
straight-line code:

    $ litton-disassembler examples/fibonacci.drum
    700: CL
         ST    0
         SK
    701: AK
         XC    1
    702: CA    1
         ST    7
    703: JM    $707
         XC    0
    704: AD    1
         JC    $706
    705: XC    1
         XC    0
    ...

There is also a raw mode that shows the sub-instructions in each word:

    $ litton-disassembler --raw examples/fibonacci.drum
    700: | CL          | ST    0     | SK          |             | NEXT: $701
    701: | AK          | XC    1     | JU    $702  |             | NEXT: $702
    702: | CA    1     | ST    7     |             |             | NEXT: $703
    703: | JM    $707  | XC    0     | NN          |             | NEXT: $704
    704: | AD    1     | JC    $706  |             |             | NEXT: $705
    705: | XC    1     | XC    0     | JU    $702  |             | NEXT: $706
    ...

## Documentation

The following documents are from Litton and explain different aspects
of the machine:

* [EBS315 ABS 1231 Operator Manual](http://www.bitsavers.org/pdf/litton/EBS315_ABS_1231_Operator_Manual_1969.pdf)
* [Litton 1600 Technical Reference Manual](http://www.bitsavers.org/pdf/litton/Litton1600_TechnicalRefMan.pdf)

Documentation for the tools in this repository:

* [Litton Drum Image File Format](doc/drum-image-format.md)
* [Litton Low-Level Cross Assembler](doc/assembler-low-level.md)

## TODO

* Support for non-ASCII character sets.
* Input and output punch tape formats.
* Front panel interface that mimics the actual machine.
* Custom OPUS implementation to get a basic OS for the emulator.
* Dump the real OPUS and integrate it when possible.

## License

Distributed under the terms of the MIT license.

## Contact

For more information on this code, to report bugs, or to suggest
improvements, please contact the author Rhys Weatherley via
[email](mailto:rhys.weatherley@gmail.com).
