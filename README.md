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

Then you can run the emulator on the examples:

    litton-run examples/hello_world.drum

Use the `-v` (verbose) option to disassemble instructions as they are executed:

    litton-run -v examples/hello_world.drum

## Documentation

The following documents are from Litton and explain different aspects
of the machine:

* [EBS315 ABS 1231 Operator Manual](http://www.bitsavers.org/pdf/litton/EBS315_ABS_1231_Operator_Manual_1969.pdf)
* [Litton 1600 Technical Reference Manual](http://www.bitsavers.org/pdf/litton/Litton1600_TechnicalRefMan.pdf)

Documentation for the tools in this repository:

* [Litton Drum Image File Format](doc/drum-image-format.md)

## TODO

* Support for non-ASCII character sets.
* Input and output punch tape formats.
* Front panel interface that mimics the actual machine.
* Disassembler for drum images and paper tapes.
* Simple assembler for creating new programs.
* Custom OPUS implementation to get a basic OS for the emulator.
* Dump the real OPUS and integrate it when possible.

## License

Distributed under the terms of the MIT license.

## Contact

For more information on this code, to report bugs, or to suggest
improvements, please contact the author Rhys Weatherley via
[email](mailto:rhys.weatherley@gmail.com).
