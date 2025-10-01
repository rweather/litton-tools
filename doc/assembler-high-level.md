Litton High-Level Cross Assembler
=================================

This document describes the high-level cross assembler, which can generate
code for the interpreted language provided by OPUS.

## Registers

* A - Primary accumulator
* B - Second accumulator
* P00 to P127 - 128 registers for storing the program
* V00 to V63 - 64 value registers for storing constants and other data
* D0 to D499 - 500 "distribution" registers for additional storage

## Numbers

Registers can hold any 36-bit integer between
-2<sup>35</sup> and +2<sup>35</sup> - 1.  This corresponds to up to 10
decimal digits.

## Instructions

Instructions are encoded as 10-bit values, packed 4 to a word.

<table border="1">
<tr><td><b>Code</b></td><td><b>Mnemonic</b></td><td><b>Name</b></td><td><b>Operand</b></td><td><b>Modifies</b></td><td><b>Description</b></td></tr>
<tr><td><tt>1</tt></td><td><tt>XCB</tt></td><td>Exchange AB</td><td> </td><td>A, B</td><td>Exchange A and B</td></tr>
<tr><td><tt>2</tt></td><td><tt>ADD</tt></td><td>Add</td><td> </td><td>A</td><td>Set A to A + B</td></tr>
<tr><td><tt>4</tt></td><td><tt>JPS</tt></td><td>Jump Positive</td><td> </td><td> </td><td>Skips the next instruction if A is positive or zero</td></tr>
<tr><td><tt>5</tt></td><td><tt>XCV</tt></td><td>Exchange V00</td><td> </td><td>A, V00</td><td>Exchange A with V00</td></tr>
<tr><td><tt>6</tt></td><td><tt>SCO</tt></td><td>Single Character Output</td><td> </td><td> </td><td>Write a single character from A to the output device with odd parity</td></tr>
<tr><td><tt>8</tt></td><td><tt>CLR</tt></td><td>Clear</td><td> </td><td>A</td><td>Clear A to zero</td></tr>
<tr><td><tt>9</tt></td><td><tt>NGA</tt></td><td>Negate A</td><td> </td><td>A</td><td>Set A to -A</td></tr>
<tr><td><tt>10</tt></td><td><tt>NGB</tt></td><td>Negate B</td><td> </td><td>A</td><td>Set B to -B</td></tr>
<tr><td><tt>14</tt></td><td><tt>SCI</tt></td><td>Single Character Input</td><td> </td><td>A</td><td>Read a single character from the input device into A</td></tr>
<tr><td><tt>30</tt></td><td><tt>SPEC</tt></td><td>Output Special Data</td><td> </td><td> </td><td>Write a single character from A to the output device with no parity control</td></tr>
<tr><td><tt> </tt></td><td><tt>BVnn</tt></td><td>Bring Vnn</td><td>0-63</td><td>A, B</td><td>Transfer A to B and bring the contents of Vnn into A</td></tr>
<tr><td><tt> </tt></td><td><tt>SVnn</tt></td><td>Store to Vnn</td><td>0-63</td><td>Vnn</td><td>Store A to Vnn</td></tr>
<tr><td><tt> </tt></td><td><tt>UVnn</tt></td><td>Update Vnn</td><td>0-63</td><td>Vnn</td><td>Set Vnn to Vnn + A</td></tr>
<tr><td><tt> </tt></td><td><tt>ACCnn</tt></td><td>Accumulate Vnn</td><td>0-63</td><td>A</td><td>Set A to A + Vnn</td></tr>
<tr><td><tt> </tt></td><td><tt>MDVnn</tt></td><td>Multiply - Divide</td><td>0-31</td><td>A</td><td>Sets A to (A * B / Vnn)</td></tr>
<tr><td><tt> </tt></td><td><tt>AJ</tt></td><td>Automatic Jump</td><td> </td><td> </td><td>Jumps to the next program register; used to pad program registers with less than 4 instructions.  P127 cannot be padded in this way.</td></tr>
<tr><td><tt> </tt></td><td><tt>JUPnnn</tt></td><td>Jump Unconditional</td><td>0-127</td><td> </td><td>Jumps to program register nnn</td></tr>
<tr><td><tt> </tt></td><td><tt>JZPnnn</tt></td><td>Jump Zero</td><td>0-127</td><td> </td><td>Jumps to program register nn if A is zero; or decrement A and continue or if A is non-zero.</td></tr>
<tr><td><tt> </tt></td><td><tt>JMKnnn</tt></td><td>Jump Mark</td><td>0-127</td><td> </td><td>Marks the current program location and jumps to program register nnn.  Control returns to the next instruction when a <tt>JR</tt> instruction is encountered.</td></tr>
<tr><td><tt> </tt></td><td><tt>JR</tt></td><td>Jump Return</td><td> </td><td> </td><td>Jumps back to the program position previously marked by <tt>JMKnnn</tt></td></tr>
<tr><td><tt> </tt></td><td><tt>SELnn</tt></td><td>Select Channels</td><td>0-63</td><td> </td><td>Selects I/O devices according to the 6-bit value nn</td></tr>
<tr><td><tt> </tt></td><td><tt>INnn</tt></td><td>Input</td><td>1-10</td><td> </td><td>B is copied to A, and up to nn digits are input into A.  V00 will be set based on the key that is used to terminate the input.</td></tr>
<tr><td><tt> </tt></td><td><tt>SKIP</tt></td><td>Skip Field From Tape</td><td> </td><td> </td><td>Reads and ignores an input field from tape, which is expected to be terminated with the "I" key.</td></tr>
<tr><td><tt> </tt></td><td><tt>OUTnn</tt></td><td>Output</td><td>0-31</td><td> </td><td>Outputs the value in A according to the formatting rules in Vnn.</td></tr>
<tr><td><tt> </tt></td><td><tt>DUPnnn</tt></td><td>Duplicate</td><td>0-190</td><td> </td><td>???</td></tr>
<tr><td><tt> </tt></td><td><tt>COnn</tt></td><td>Character Output</td><td>0-63</td><td> </td><td>Outputs the literal character code nn</td></tr>
<tr><td><tt> </tt></td><td><tt>TABnnn</tt></td><td>Tab</td><td>1-190</td><td> </td><td>Tab across to position nnn on the printer</td></tr>
<tr><td><tt> </tt></td><td><tt>ALFI</tt></td><td>Alphanumeric Input</td><td> </td><td>A</td><td>Reads 5 raw characters into A</td></tr>
<tr><td><tt> </tt></td><td><tt>ALFO</tt></td><td>Alphanumeric Output</td><td> </td><td> </td><td>Writes 5 raw characters from A to the selected output</td></tr>
<tr><td><tt> </tt></td><td><tt>INA</tt></td><td>Input from ASCII-coded tape</td><td> </td><td>A</td><td>Reads a number from tape in ASCII format into A</td></tr>
<tr><td><tt> </tt></td><td><tt>DCLR</tt></td><td>Clear Distribution Registers</td><td> </td><td>D[All]</td><td>Clear all 500 distribution registers to zero</td></tr>
<tr><td><tt> </tt></td><td><tt>DGET</tt></td><td>Bring a Distribution Register</td><td> </td><td>A, B</td><td>Copy A into B and then bring the content of the distribution register with address V07 into A</td></tr>
<tr><td><tt> </tt></td><td><tt>DPUT</tt></td><td>Store to a Distribution Register</td><td> </td><td>D[V07]</td><td>Store A to the distribution register with address V07</td></tr>
<tr><td><tt> </tt></td><td><tt>SCAN</tt></td><td>Scan for a Non-Zero Distribution Register</td><td> </td><td>A, V07</td><td>Scans the distribution registers starting at V07 for a non-zero value.  The value is copied into A and V07 is updated to contain the address of the non-zero value.  If V07 is greater than 499 after this instruction, there are no more non-zero values.  Cannot be the fourth instruction of a program word.</td></tr>
<tr><td><tt> </tt></td><td><tt>DIST</tt></td><td>Distribute</td><td> </td><td> </td><td>???</td></tr>
<tr><td><tt> </tt></td><td><tt>SGET</tt></td><td>Load Split Distribution Register</td><td> </td><td>A, B</td><td>Loads the distribution register whose address is stored in V07 and splits it into A and B.</td></tr>
<tr><td><tt> </tt></td><td><tt>SPUT</tt></td><td>Store Split Distribution Register</td><td> </td><td>A</td><td>Combines A and B and stores the value to the distribution register whose address is stored in V07.  A is set to the combined value.</td></tr>
<tr><td><tt> </tt></td><td><tt>OPUS</tt></td><td>Program Interrupt</td><td> </td><td> </td><td>Jump back into OPUS for diagnostic purposes</td></tr>
<tr><td><tt> </tt></td><td><tt>CALC</tt></td><td>Calculate</td><td> </td><td> </td><td>???</td></tr>
<tr><td><tt> </tt></td><td><tt>CDV</tt></td><td>Check-Digit Verification</td><td> </td><td> </td><td>???</td></tr>
<tr><td><tt> </tt></td><td><tt>DUPE</tt></td><td>Duplicate Data With Even Parity</td><td> </td><td> </td><td>???</td></tr>
<tr><td><tt> </tt></td><td><tt>DUPO</tt></td><td>Duplicate Data With Odd Parity</td><td> </td><td> </td><td>???</td></tr>
</table>

## Input field terminators

When a value is input using the "IN" instruction, the user can press a
control key to terminate the field.  V00 will be set based on the key
that is pressed to allow the program to determine which control key it was.

<table border="1">
<tr><td><b>Key</b></td><td><b>V00</b></td></tr>
<tr><td>I</td><td>Not changed</td></tr>
<tr><td>II</td><td>1</td></tr>
<tr><td>III</td><td>2</td></tr>
<tr><td>IIII</td><td>3</td></tr>
<tr><td>P0 / @</td><td>4</td></tr>
<tr><td>P1</td><td>5</td></tr>
<tr><td>P2</td><td>6</td></tr>
<tr><td>P3</td><td>7</td></tr>
<tr><td>P4</td><td>8</td></tr>
</table>
