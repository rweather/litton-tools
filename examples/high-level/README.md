Example high-level OPUS programs
================================

## Loading and running the programs

Run the `litton` GUI emulator with no arguments to launch into OPUS.
Press HALT, READY, RUN in that order to start running OPUS.

Then press F5 (F1) to start loading a program tape.  The "TAPE IN"
button will highlight.  Press it and select the tape you want to load.
After loading, you can press F4 (IIII) to run the program.

## Fibonacci

The program `fibonacci.tape` calculates the Fibonacci sequence and
prints it one number at a time to the printer.

## Print Paper Tape

The program `print-tape.tape` prints the contents of a paper tape to the
printer.  Load `print-tape.tape` into the emulator and press F4 (IIII) to run.

The "TAPE IN" button will highlight.  Press the button and select the
tape file that you want to print.

Once the input tape is exhausted, the program will spin in an infinite loop
waiting for the next tape symbol.  Press HALT to stop the program.

This program was found on the drum for David Lovett's Litton computer.
It was apparently the last program the previous owner ran before the
machine was decommisioned.
