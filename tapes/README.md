Archived Litton Tapes
=====================

This directory contains program tape images for the Litton minicomputer
from the collection of David Lovett a.k.a Usagi Electric.

The `.tape` files are formatted in ASCII for the Litton emulator.
The `.bin` files are the binary versions, suitable for punching back
onto paper tape to be fed into a real Litton with a real tape reader.

Instructions are given below for how to run the programs with the
emulator.

Some minor fixups were performed on the tape images.  Blackjack originally
had to be loaded in two sections, but the image in this archive can do it
in a single load.

## Blackjack

`blackjack.tape` is a program that plays [Blackjack](https://en.wikipedia.org/wiki/Blackjack).

A partial copy of Blackjack was found on the drum of David Lovett's Litton,
but it wasn't complete.  David then found the original tape in his collection,
so we were able to restore it.

Blackjack works best with the command-line version of the emulator so that
red ribbon printing works.  Start the program with the emulator as follows:

    litton-run -i blackjack.tape

Press F5 to load the tape and then type the following commands to run it:

    H
    J 300#

The program will print some instructions.  When it says "SHUFFLING",
press F1 to stop shuffling.  Then it will ask you to "NOW CUT".
Press F1 again to cut.  The program is probably using the user's
keyboard input timing to seed a random number generator.

During play, the player's hand and the house's hand will be displayed.
Press F1 to deal or request another card with "hit".  Press Space to pass.
Once the player wins or goes bust, press F1 again for another hand.

## Duptape

`duptape.tape` is a program that duplicates tapes, reading from the
tape reader and writing to the tape punch.

## Lucy

`lucy.tape` is a program that prints a calendar for 1973, featuring the
character "Lucy" from the Charlie Brown comics.

Because the output is so long, it is best to run the program using the
command-line version of the emulator:

    litton-run -i lucy.tape

Once the emulator starts, press F5 to load the tape and then F4 to run it.
Press CTRL-C to exit.

A dump of the program's output can be found in [lucy.txt](lucy.txt).
The text is 173 columns wide, designed to be printed on the Litton's
original 192-column printer.

## Other tapes, yet to be analysed

These look like business software from the original owner.  These are
still being analysed to see what they do.

* `102testtape.tape` - ???, looks like test data rather than a program.
* `132III-C.tape` - ???
* `dpma.tape` - Depreciation calculator?
* `f15pt.tape` - ???
