
The "Litton-Emulator" directory contains an Arduino sketch for
running the Litton emulator on an Arduino Mega 2560 or better.
Note: Arduino Uno doesn't have enough memory.

Copy the "Litton-Emulator" directory and all of its contents to the
"sketchbook" directory in your home directory.  Then restart the
Arduino IDE and select "File -> Sketchbook -> Litton-Emulator".

Minicom is recommended to talk to the Arduino.  Configure it as follows:

* Serial port setup: "Serial Device" should be set to "/dev/ttyACM0" or
whatever your port is.  "Bps/Par/Bits" should be set to 115200 8N1.
"Hardware Flow Control" should be off.
* Use CTRL-A, T to select "Terminal settings".  Set the "Character tx delay"
to 10 milliseconds.  Unfortunately this setting does not save in minicom
so you need to set it every time.
* Use CTRL-A, W to turn on line wrapping.  Unfortunately this setting
does not save in minicom so you need to set it every time.

Connect to the Arduino.  You should see "READY" and then a CRLF for
the OPUS prompt.  If not, press the Reset button on the Arduino.

To load a tape image, use CTRL-Y.  When you see "TAPE READER ON",
paste in the characters from the `.tape` file.  The final comma should
result in "TAPE READER OFF".  If this doesn't work, make sure you have
set the "Character tx delay" correctly.

If the tape image is for a high-level assembly program, then use
CTRL-T to run it.  If the tape image is for a low-level machine code
program, then use H followed by "J 800#" to run it, substituting the
start address for "800".

Pressing CTRL-C will halt the machine and drop back into OPUS.
This is equivalent to pressing HALT, READY, RUN on the real Litton.

Press '?' for help on OPUS commands when running the emulator.
