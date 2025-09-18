Litton Low-Level Cross Assembler
================================

This document describes the low-level cross assembler, which can generate
machine code for the Litton minicomputer.  This is different from the
"high-level" assembler that is provided by OPUS and supporting software
for writing user-facing programs.

The Litton has a very weird machine code format.  Every 40-bit word has an
implicit jump destination in the most significant byte which can jump
anywhere in the local 256-word page of memory.  Between 2 and 4
instructions are packed into the 4 least significant bytes.
Jumping to another page requires an explicit 2-byte jump instruction
in the 4 least significant bytes.

The assembler hides this detail from the user.  The user can write
straight-line assembly code without needing to worry about packing
instructions, setting up the implicit jumps, or putting in explicit
jumps to cross a page boundary.  If necessary, the assembler will add
no-op instructions to pad instruction words, and explicit jumps
across page boundaries.

## Using the Assembler

The assembler takes in a ".las" (Litton Assembler Source) file and
outputs a ".drum" (Litton Drum Image) file:

    litton-as -o output.drum input.las

If the source does not contain a "title" directive, then the title can be
supplied on the command-line:

    litton-as -t "My Program" -o myprog.drum myprog.las

## Line Format

Lines start with an optional label, followed by an instruction/directive
name, operands, and finally an optional comment.

Labels are identifiers that start in column 1 of a line.  If there is any
whitespace before the identifier, it is an instruction opcode or directive
instead.  A label may optionally end in a colon (:).

In the example below, the first left-aligned "JU" is a label called "JU".
The second "JU" is the opcode for "jump unconditional".  The third "JU"
refers to the first label, setting up an infinite loop.

    JU JU JU

## Identifiers

Identifiers start with any of A-Z, a-z, period (.), or underscore (\_).
The rest of the identifier may be A-Z, a-z, 0-9, period (.), or
underscore (\_).  The following are examples of identifiers:

    Fred
    .Mary
    A_27
    _42
    dot.this.dot.that

All identifiers except instruction opcode and directive names are
case-sensitive.  Opcode and directive names may be provided in either case.

## Numbers

Numbers may be decimal, binary, octal, or hexadecimal, as determined by the
prefix or lack thereof on the number.  The following all refer to the same
value:

    42          ; Decimal
    %00101010   ; Binary
    @52         ; Octal
    $2A         ; Hexadecimal

Negative numbers are also possible, with no space between the minus
sign and the digit:

    -42         ; Decimal
    %-00101010  ; Binary
    @-52        ; Octal
    $-2A        ; Hexadecimal
    - 42        ; Invalid!

40-bit words allow any number between -549755813888 and +1099511627775.
Negative numbers are represented in twos-complement form.

## Strings

Strings may be used to refer to individual characters or a sequences of bytes.
They may be surrounded by single or double quotes.

    "Hello, World!"
    'H'
    '"'

There are some limited escape sequences that are supported:

    \a
    \b
    \f
    \n
    \r
    \t
    \v

Note: It is not possible to escape the quote at the end of a string,
so '\'' and "\"" are not valid.  Use "'" and '"' instead.

If a string extends to the end of the current line, it will be treated
as though an implicit ending quote was supplied.

The assembler converts the string into the active character set,
as selected by the `.charset` or `.console` directives.

The default EBS1231 character set has a number of codes for special
keys on the keyboard or for moving the print head to a specific position.
These can be expressed with strings like "[P1]" for the P1 key or
"{49}" for moving the print head to position 49.

* "\r" - Return
* "\n" - Line Feed Left
* "\f" - Form Up
* "\b" - Backspace
* "[P1]", "[P2]", "[P3]", "[P4]"
* "[I]", "[II]", "[III]", "[IIII]"
* "[LFB]" - Line Feed Both
* "[LFR]" - Line Feed Right
* "[BR]" - Black Ribbon Print
* "[RR]" - Red Ribbon Print
* "[TL]" - Tape Leader / Carriage Open or Close
* "{n}" - Move print position to n, where n is between 4 and 190 in steps of 3

## Comments

Comments start with a semi-colon ";" and extend to the end of the
current line.  For example:

    SCRATCH equ $400    ; Scratch space for global variables.

## Directives

The following assembler directives are provided:

* `title "TITLE"` - Set the title of the drum image to `TITLE`.  This may be
overridden by the `-t` command-line option.
* `org N` - Sets the origin for the code that follows to N.
* `LABEL equ N` - Sets `LABEL` to the value of N.  N can be a literal
number or a reference to another label that was already defined.
This can also be written as `LABEL = N`.
* `dw N1, N2, ...` - Outputs 40-bit words into the instruction stream with
values N1, N2, etc.  If a value is a string, it will be split into one
character per word.
* `db N1, N2, ...` - Outputs 8-bit bytes into the instruction stream with
values N1, N2, etc.  The bytes are packed starting at the most significiant
bit of the 40-bit word on down.  Left-over bytes are filled with zeroes.
* `entry N` - Sets the entry point for the program to N.  Can only appear
once in the source file.
* `printer N,"CHARSET"` - Sets the printer device identifier to N and the
printer character set to `CHARSET`.  This will also define the symbol
`printer` to be N and change the string character set to `CHARSET`.
Can only appear once in the source file.
* `keyboard N,"CHARSET"` - Sets the keyboard device identifier to N and the
keyboard character set to `CHARSET`.  This will also define the symbol
`keyboard` to be N.  Can only appear once in the source file.
* `charset "CHARSET"` - Sets the character set to use for the
following strings.  The default character set is "EBS1231".
* `align` - Aligns the next instruction on a word boundary, inserting
no-op's as necessary.  This is done implicitly for code labels as it is only
possible to jump to word-aligned addresses.
* `drumsize N` - Sets the size of the drum to N, usually 2048 or 4096.
Minimum is 64, maximum is 4096.  Default is 4096.

Directive names are not case-sensitive.

## Core Instructions

All of the instructions from section 3.7 of the
[Litton 1600 Technical Reference Manual](http://www.bitsavers.org/pdf/litton/Litton1600_TechnicalRefMan.pdf)
are supported.

The machine has the following registers:

* A - 40-bit accumulator
* B - 8-bit buffer to assist in I/O operations
* CR - 8-bit command register
* I - 40-bit instruction register
* K - 1-bit carry flag
* P - 1-bit parity failure bit: latches to 1 when a parity error is seen and can only be reset to 0 by a `TP` instruction.
* Sn - 40-bit scratchpad register n (0-7); can also be accessed as address $00n in main memory.

The 48-bit combination of CR and I provides the current instruction to be
executed.  When an instruction completes, the value in CR and I is rotated
left to bring the next instruction opcode into CR.

Some of the shift instructions below refer to the "available scratchpad
register".  It is currently unclear which scratchpad register this is.

<table border="1">
<tr><td><b>Hex Code</b></td><td><b>Mnemonic</b></td><td><b>Full Name</b></td><td><b>Operand</b></td><td><b>Modifies</b></td><td><b>Description</b></td></tr>
<tr><td><tt>00+X</tt></td><td><tt>HH X</tt></td><td><tt>HALT</tt></td><td>0-7</td><td> </td><td>Halt the machine with X displayed on the indicator lights</td></tr>
<tr><td><tt>08</tt></td><td><tt>AK</tt></td><td><tt>ADD K</tt></td><td> </td><td>A, K</td><td>Adds the contents of K to A; K is set on overflow</td></tr>
<tr><td><tt>09</tt></td><td><tt>CL</tt></td><td><tt>CLEAR A</tt></td><td> </td><td>A</td><td>Clear A to zero</td></tr>
<tr><td><tt>0A</tt></td><td><tt>NN</tt></td><td><tt>NO OPERATION</tt></td><td> </td><td> </td><td>No operation</td></tr>
<tr><td><tt>0B</tt></td><td><tt>CM</tt></td><td><tt>COMPLEMENT</tt></td><td> </td><td>A, K</td><td>Two's complement of A; K is set to 1 if A is non-zero</td></tr>
<tr><td><tt>0D</tt></td><td><tt>JA</tt></td><td><tt>JUMP TO A</tt></td><td> </td><td>I</td><td>Copy the contents of A into I</td></tr>
<tr><td><tt>0F</tt></td><td><tt>BI</tt></td><td><tt>BLOCK INTERCHANGE</tt></td><td> </td><td>S0-S7</td><td>Exchange the scratchpad with the block interchange loop</td></tr>
<tr><td><tt>10</tt></td><td><tt>SK</tt></td><td><tt>SET K TO 1</tt></td><td> </td><td>K</td><td>Set K to 1</td></tr>
<tr><td><tt>11</tt></td><td><tt>TZ</tt></td><td><tt>TEST FOR ZERO</tt></td><td> </td><td>K</td><td>Sets K to 1 if A is 0; set K to 0 otherwise</td></tr>
<tr><td><tt>12</tt></td><td><tt>TH</tt></td><td><tt>TEST HIGH ORDER A BIT</tt></td><td> </td><td>K</td><td>Sets K to 1 if the most signficant bit of A is 1; set K to 0 otherwise</td></tr>
<tr><td><tt>12</tt></td><td><tt>TN</tt></td><td><tt>TEST FOR NEGATIVE</tt></td><td> </td><td>K</td><td>Sets K to 1 if the most signficant bit of A is 1; set K to 0 otherwise (alias for <tt>TH</tt>)</td></tr>
<tr><td><tt>13</tt></td><td><tt>RK</tt></td><td><tt>RESET K TO 0</tt></td><td> </td><td>K</td><td>Set K to 0</td></tr>
<tr><td><tt>14</tt></td><td><tt>TP</tt></td><td><tt>TEST PARITY FAILURE</tt></td><td> </td><td>K</td><td>Set K to the value of P and then reset P to 0</td></tr>
<tr><td><tt>18+N</tt></td><td><tt>LA N</tt></td><td><tt>LOGICAL AND</tt></td><td>0-7</td><td>A, K</td><td>Set A to the result of AND'ing A with Sn; K is set to 1 if the result is non-zero</td></tr>
<tr><td><tt>20+N</tt></td><td><tt>XC N</tt></td><td><tt>EXCHANGE</tt></td><td>0-7</td><td>A, Sn</td><td>Exchanges A with Sn</td></tr>
<tr><td><tt>28+N</tt></td><td><tt>XT N</tt></td><td><tt>EXTRACT</tt></td><td>0-7</td><td>A, Sn</td><td>Simultaneously set A to (Sn AND A) and Sn to (Sn AND NOT A)</td></tr>
<tr><td><tt>30+N</tt></td><td><tt>TE N</tt></td><td><tt>TEST EQUAL</tt></td><td>0-7</td><td>K</td><td>Set K to 1 if A is equal to Sn; set K to 0 otherwise</td></tr>
<tr><td><tt>38+N</tt></td><td><tt>TG N</tt></td><td><tt>TEST EQUAL OR GREATER</tt></td><td>0-7</td><td>K</td><td>Set K to 1 if A is greater than or equal to Sn; set K to 0 otherwise</td></tr>
<tr><td><tt>4000+(N-1)</tt></td><td><tt>BLS N</tt></td><td><tt>BINARY LEFT SINGLE SHIFT</tt></td><td>1-128</td><td>A, K</td><td>Shift A left by N bits with K set to the final carry out from the high bit</td></tr>
<tr><td><tt>4080+(N-1)</tt></td><td><tt>BLSK N</tt></td><td><tt>BINARY LEFT SINGLE SHIFT INCLUDING K</tt></td><td>1-128</td><td>A, K</td><td>Rotate A and K left by N bits with K set to the final carry out from the high bit</td></tr>
<tr><td><tt>4100</tt></td><td><tt>BLSS</tt></td><td><tt>BINARY LEFT SINGLE SHIFT ON SCRATCHPAD</tt></td><td> </td><td>S, K</td><td>Shift the available scratchpad register left by 1 bit with K set to the final carry out from the high bit of S</td></tr>
<tr><td><tt>4180</tt></td><td><tt>BLSSK</tt></td><td><tt>BINARY LEFT SINGLE SHIFT ON SCRATCHPAD INCLUDING K</tt></td><td> </td><td>S, K</td><td>Rotate the available scratchpad register and K left by 1 bit with K set to the final carry out from the high bit</td></tr>
<tr><td><tt>4200+(N-1)</tt></td><td><tt>BLD N</tt></td><td><tt>BINARY LEFT DOUBLE SHIFT</tt></td><td>1-128</td><td>S0, S1, K</td><td>Shift S0/S1 left by N bits with K set to the final carry out from the high bit</td></tr>
<tr><td><tt>4280+(N-1)</tt></td><td><tt>BLDK N</tt></td><td><tt>BINARY LEFT DOUBLE SHIFT INCLUDING K</tt></td><td>1-128</td><td>S0, S1, K</td><td>Rotate S0/S1 and K left by N bits with K set to the final carry out from the high bit</td></tr>
<tr><td><tt>4300</tt></td><td><tt>BLDS</tt></td><td><tt>BINARY LEFT DOUBLE SHIFT ON SCRATCHPAD</tt></td><td> </td><td>S, K</td><td>Shift the available double scratchpad register left by 1 bit with K set to the carry out from the high bit</td></tr>
<tr><td><tt>4380</tt></td><td><tt>BLDSK</tt></td><td><tt>BINARY LEFT DOUBLE SHIFT ON SCRATCHPAD INCLUDING K</tt></td><td> </td><td>S, K</td><td>Rotate the available double scratchpad register and K left by 1 bit with K set to the final carry out from the high bit</td></tr>
<tr><td><tt>4800+(N-1)</tt></td><td><tt>BRS N</tt></td><td><tt>BINARY RIGHT SINGLE SHIFT</tt></td><td>1-128</td><td>A, K</td><td>Shift A right by N bits with K set to the final carry out from the low bit</td></tr>
<tr><td><tt>4880+(N-1)</tt></td><td><tt>BRSK N</tt></td><td><tt>BINARY RIGHT SINGLE SHIFT INCLUDING K</tt></td><td>1-128</td><td>A, K</td><td>Rotate A and K right by N bits with K set to the final carry out from the low bit</td></tr>
<tr><td><tt>4900</tt></td><td><tt>BRSS</tt></td><td><tt>BINARY RIGHT SINGLE SHIFT ON SCRATCHPAD</tt></td><td> </td><td>S, K</td><td>Shift the available scratchpad register right by 1 bit with K set to the final carry out from the low bit</td></tr>
<tr><td><tt>4980</tt></td><td><tt>BRSSK</tt></td><td><tt>BINARY RIGHT SINGLE SHIFT ON SCRATCHPAD INCLUDING K</tt></td><td> </td><td>S, K</td><td>Rotate the available scratchpad register and K right by 1 bit with K set to the final carry out from the low bit</td></tr>
<tr><td><tt>4A00+(N-1)</tt></td><td><tt>BRD N</tt></td><td><tt>BINARY RIGHT DOUBLE SHIFT</tt></td><td>1-128</td><td>S0, S1, K</td><td>Shift S0/S1 right by N bits with K set to the final carry out from the low bit</td></tr>
<tr><td><tt>4A80+(N-1)</tt></td><td><tt>BRDK N</tt></td><td><tt>BINARY RIGHT DOUBLE SHIFT INCLUDING K</tt></td><td>1-128</td><td>S0, S1, K</td><td>Rotate S0/S1 and K left by N bits with K set to the final carry out from the low bit</td></tr>
<tr><td><tt>4B00</tt></td><td><tt>BRDS</tt></td><td><tt>BINARY RIGHT DOUBLE SHIFT ON SCRATCHPAD</tt></td><td> </td><td>S, K</td><td>Shift the available double scratchpad register right by 1 bit with K set to the carry out from the low bit</td></tr>
<tr><td><tt>4B80</tt></td><td><tt>BRDSK</tt></td><td><tt>BINARY RIGHT DOUBLE SHIFT ON SCRATCHPAD INCLUDING K</tt></td><td> </td><td>S, K</td><td>Rotate the available double scratchpad register and K right by 1 bit with K set to the final carry out from the low bit</td></tr>
<tr><td><tt>5000</tt></td><td><tt>SI</tt></td><td><tt>SHIFT INPUT</tt></td><td> </td><td>A, B, K, P</td><td>If the selected input device is ready, then set K to 1 and read a byte from the device.  The byte is shifted into the low bits of A and the former high bits of A are shifted into B.  P may be set, but usually <tt>SI</tt> is used to input without parity.  If the selected input device is busy, then K is set to 0 only.</td></tr>
<tr><td><tt>5080</tt></td><td><tt>RS</tt></td><td><tt>READ STATUS</tt></td><td> </td><td>A, B, K</td><td>If the selected input device is ready, then set K to 1 and read the status from the device.  The status is shifted into the low bits of A and the former high bits of A are shifted into B.  If the selected input device is busy, then K is set to 0 only.</td></tr>
<tr><td><tt>5800</tt></td><td><tt>CIO</tt></td><td><tt>CLEAR A, INPUT WITH ODD PARITY</tt></td><td> </td><td>A, B, K, P</td><td>If the selected input device is ready, then set K to 1 and read a byte from the device into B.  The low bits of A are set to B and the rest of the bits are set to 0.  P will be set to 1 if the byte has an odd parity error.  If the selected input device is busy, then K is set to 0 only.</td></tr>
<tr><td><tt>5840</tt></td><td><tt>CIE</tt></td><td><tt>CLEAR A, INPUT WITH EVEN PARITY</tt></td><td> </td><td>A, B, K, P</td><td>If the selected input device is ready, then set K to 1 and read a byte from the device into B.  The low bits of A are set to B and the rest of the bits are set to 0.  P will be set to 1 if the byte has an even parity error.  If the selected input device is busy, then K is set to 0 only.</td></tr>
<tr><td><tt>5C00</tt></td><td><tt>CIOP</tt></td><td><tt>CLEAR A, INPUT WITH ODD PARITY INTO A</tt></td><td> </td><td>A, B, K, P</td><td>Same as <tt>CIO</tt> except that the state of P is also recorded into the high bit of A.</td></tr>
<tr><td><tt>5C40</tt></td><td><tt>CIEP</tt></td><td><tt>CLEAR A, INPUT WITH EVEN PARITY INTO A</tt></td><td> </td><td>A, B, K, P</td><td>Same as <tt>CIE</tt> except that the state of P is also recorded into the high bit of A.</td></tr>
<tr><td><tt>6000+(N-1)</tt></td><td><tt>DLS N</tt></td><td><tt>DECIMAL LEFT SINGLE SHIFT</tt></td><td>1-128</td><td>A, K</td><td>Set A to the result of multiplying A by ten N times; K is destroyed.</td></tr>
<tr><td><tt>6080+(N-1)</tt></td><td><tt>DLSC N</tt></td><td><tt>DECIMAL LEFT SINGLE SHIFT, PLUS CONSTANT</tt></td><td>1-128</td><td>A, K</td><td>Set A to the result of multiplying (A+1) by ten N times; K is destroyed.</td></tr>
<tr><td><tt>6100</tt></td><td><tt>DLSS</tt></td><td><tt>DECIMAL LEFT SINGLE SHIFT ON SCRATCHPAD</tt></td><td> </td><td>S, K</td><td>Set the available scratchpad register S to S multiplied by ten; K is destroyed.</td></tr>
<tr><td><tt>6180</tt></td><td><tt>DLSSC</tt></td><td><tt>DECIMAL LEFT SINGLE SHIFT ON SCRATCHPAD, PLUS CONSTANT</tt></td><td> </td><td>S, K</td><td>Set the available scratchpad register S to (S+1) multiplied by ten; K is destroyed.</td></tr>
<tr><td><tt>6200+(N-1)</tt></td><td><tt>DLD N</tt></td><td><tt>DECIMAL LEFT DOUBLE SHIFT</tt></td><td>1-128</td><td>S0, S1, K</td><td>Set S0/S1 to the result of multiplying S0/S1 by ten N times; K is destroyed.</td></tr>
<tr><td><tt>6280+(N-1)</tt></td><td><tt>DLDC N</tt></td><td><tt>DECIMAL LEFT DOUBLE SHIFT, PLUS CONSTANT</tt></td><td>1-128</td><td>S0, S1, K</td><td>Set S0/S1 to the result of multiplying (S0/S1+1) by ten N times; K is destroyed.</td></tr>
<tr><td><tt>6300</tt></td><td><tt>DLDS</tt></td><td><tt>DECIMAL LEFT DOUBLE SHIFT ON SCRATCHPAD</tt></td><td> </td><td>S, K</td><td>Set the available scratchpad double register S to the result of multiplying S by ten; K is destroyed.</td></tr>
<tr><td><tt>6380</tt></td><td><tt>DLDSC</tt></td><td><tt>DECIMAL LEFT DOUBLE SHIFT ON SCRATCHPAD, PLUS CONSTANT</tt></td><td> </td><td>S, K</td><td>Set the available scratchpad double register S to the result of multiplying (S+1) by ten; K is destroyed.</td></tr>
<tr><td><tt>6800+(N-1)</tt></td><td><tt>DRS N</tt></td><td><tt>DECIMAL RIGHT SINGLE SHIFT</tt></td><td>1-128</td><td>A, K</td><td>Set A to the result of dividing A by ten N times; K is destroyed.</td></tr>
<tr><td><tt>6A00+(N-1)</tt></td><td><tt>DRD N</tt></td><td><tt>DECIMAL RIGHT DOUBLE SHIFT</tt></td><td>1-128</td><td>S0, S1, K</td><td>Set S0/S1 to the result of dividing S0/S1 by ten N times; K is destroyed.</td></tr>
<tr><td><tt>7000</tt></td><td><tt>OAO</tt></td><td><tt>OUTPUT ACCUMULATOR WITH ODD PARITY</tt></td><td> </td><td>A, B, K</td><td>If the selected output device is ready, then set K to 1, generate an odd parity bit into the high bit of A, rotate the high bits of A into B, and then output B.  A is shifted left by 8 bits in the process, with 0 bits shifted in.  If the selected output device is busy, then K is set to 0 only.</td></tr>
<tr><td><tt>7040</tt></td><td><tt>OAE</tt></td><td><tt>OUTPUT ACCUMULATOR WITH EVEN PARITY</tt></td><td> </td><td>A, B, K</td><td>If the selected output device is ready, then set K to 1, generate an even parity bit into the high bit of A, rotate the high bits of A into B, and then output B.  A is shifted left by 8 bits in the process, with 0 bits shifted in.  If the selected output device is busy, then K is set to 0 only.</td></tr>
<tr><td><tt>70C0</tt></td><td><tt>OA</tt></td><td><tt>OUTPUT ACCUMULATOR WITH NO PARITY</tt></td><td> </td><td>B, K</td><td>If the selected output device is ready, then set K to 1, rotates the high bits of A into B, and then output B.  A is shifted left by 8 bits in the process, with 0 bits shifted in.  If the selected output device is busy, then K is set to 0 only.</td></tr>
<tr><td><tt>74C0</tt></td><td><tt>AST</tt></td><td><tt>ACCUMULATOR SELECT ON TEST</tt></td><td> </td><td>B, K</td><td>If no output devices are busy, then set K to 1, rotae the high bits of A into B, and then send B as a device select code on the I/O bus.  A is shifted left by 8 bits in the process, with 0 bits shifted in.  If any output device is busy, then K is set to 0 only.</td></tr>
<tr><td><tt>76C0</tt></td><td><tt>AS</tt></td><td><tt>ACCUMULATOR SELECT</tt></td><td> </td><td>B, K</td><td>Set K to 1, rotate the high bits of A into B, and then send B as a device select code on the I/O bus.  A is shifted left by 8 bits in the process, with 0 bits shifted in.  Busy devices are ignored.</td></tr>
<tr><td><tt>7800+C</tt></td><td><tt>OI C</tt></td><td><tt>OUTPUT IMMEDIATE</tt></td><td>0-255</td><td>B, K</td><td>If the selected output device is ready, then set K to 1, set B to C, and then output B.  If the selected output device is busy, then K is set to 0 only.  Note: If parity is needed, it must be included in the C value.</td></tr>
<tr><td><tt>7C00+D</tt></td><td><tt>IST D</tt></td><td><tt>IMMEDIATE SELECT ON TEST</tt></td><td>0-255</td><td>B, K</td><td>If no output devices are busy, then set K to 1, set B to D, and then send B as a device select code on the I/O bus.  If any output device is busy, then K is set to 0 only.</td></tr>
<tr><td><tt>7E00+D</tt></td><td><tt>IS D</tt></td><td><tt>IMMEDIATE SELECT</tt></td><td>0-255</td><td>B, K</td><td>Set K to 1, set B to D, and then send B as a device select code on the I/O bus.  Busy devices are ignored.</td></tr>
<tr><td><tt>8000+M</tt></td><td><tt>CA M</tt></td><td><tt>CLEAR AND ADD</tt></td><td>$000-$FFF</td><td>A</td><td>Clears the accumulator and adds the contents of memory location M to it.  That is, this instruction loads the contents of M into A.</td></tr>
<tr><td><tt>9000+M</tt></td><td><tt>AD M</tt></td><td><tt>ADD</tt></td><td>$000-$FFF</td><td>A, K</td><td>Adds the contents of memory location M to A, with the carry-out in K.  Note: carry-in is handled by the <tt>AK</tt> instruction.</td></tr>
<tr><td><tt>B000+M</tt></td><td><tt>ST M</tt></td><td><tt>STORE</tt></td><td>$000-$FFF</td><td> </td><td>Stores the contents of A to memory location M.</td></tr>
<tr><td><tt>C000+M</tt></td><td><tt>JM M</tt></td><td><tt>JUMP MARK</tt></td><td>$000-$FFF</td><td>A</td><td>Stores the contents of I into A and then jump to M.</td></tr>
<tr><td><tt>D000+M</tt></td><td><tt>AC M</tt></td><td><tt>ADD CONDITIONAL</tt></td><td>$000-$FFF</td><td>A, K</td><td>If K is set to 1, then add the contents of memory location M to A with carry-out in K.  Does nothing if K is set to 0.</td></tr>
<tr><td><tt>E000+M</tt></td><td><tt>JU M</tt></td><td><tt>JUMP UNCONDITIONAL</tt></td><td>$000-$FFF</td><td> </td><td>Jumps unconditionally to M.</td></tr>
<tr><td><tt>F000+M</tt></td><td><tt>JC M</tt></td><td><tt>JUMP CONDITIONAL</tt></td><td>$000-$FFF</td><td> </td><td>Jumps to M if K is set to 1.</td></tr>
</table>

## Pseudo Instructions

The following instructions are conveniences that expand to sequences of
core instructions:

<table border="1">
<tr><td><b>Mnemonic</b></td><td><b>Full Name</b></td><td><b>Operand</b></td><td><b>Modifies</b></td><td><b>Description</b></td></tr>
<tr></td><td><tt>ISW D</tt></td><td><tt>IMMEDIATE SELECT AND WAIT</tt></td><td>0-255</td><td>B, K</td><td>If no output devices are busy, then set K to 1, set B to D, and then send B as a device select code on the I/O bus.  If any output device is busy, then wait until all of them are ready.</td></tr>
<tr><td><tt>OAOW</tt></td><td><tt>OUTPUT ACCUMULATOR WITH ODD PARITY AND WAIT</tt></td><td> </td><td>A, B, K</td><td>If the selected output device is ready, then set K to 1, generate an odd parity bit into the high bit of A, set B to the high bits of A, and then output B.  If the selected output device is busy, wait until it is ready.</td></tr>
<tr><td><tt>OAEW</tt></td><td><tt>OUTPUT ACCUMULATOR WITH EVEN PARITY AND WAIT</tt></td><td> </td><td>A, B, K</td><td>If the selected output device is ready, then set K to 1, generate an even parity bit into the high bit of A, set B to the high bits of A, and then output B.  If the selected output device is busy, wait until it is ready.</td></tr>
<tr><td><tt>OAW</tt></td><td><tt>OUTPUT ACCUMULATOR AND WAIT</tt></td><td> </td><td>B, K</td><td>If the selected output device is ready, then set K to 1, set B to the high bits of A, and then output B.  If the selected output device is busy, then wait until it is ready.</td></tr>
<tr><td><tt>OIOW C</tt></td><td><tt>OUTPUT IMMEDIATE WITH ODD PARITY AND WAIT</tt></td><td>0-255</td><td>B, K</td><td>If the selected output device is ready, then set K to 1, set B to C, and then output B.  If the selected output device is busy, then wait until it is ready.  Odd parity is calculated automatically for C.</td></tr>
<tr><td><tt>OIEW C</tt></td><td><tt>OUTPUT IMMEDIATE WITH EVEN PARITY AND WAIT</tt></td><td>0-255</td><td>B, K</td><td>If the selected output device is ready, then set K to 1, set B to C, and then output B.  If the selected output device is busy, then wait until it is ready.  Even parity is calculated automatically for C.</td></tr>
<tr><td><tt>OIW C</tt></td><td><tt>OUTPUT IMMEDIATE AND WAIT</tt></td><td>0-255</td><td>B, K</td><td>If the selected output device is ready, then set K to 1, set B to C, and then output B.  If the selected output device is busy, then wait until it is ready.</td></tr>
</table>

## Subroutines

To call a subroutine, use the <tt>JUMP MARK</tt> instruction.
<tt>JM M</tt> copies the instruction at the call point into the A register
and then jumps to M.  The subroutine can save A into a temporary memory
location and jump to it when the subroutine returns:

        JM label
        ...
    label:
        ST temp
        ...
        JU temp
    temp:
        DW 0

The code can be made shorter using <tt>XC</tt> to save the return address
in a scratchpad register by exchanging it with A:

        JM label
        ...
    label:
        XC 7
        ...
        JU 7

Alternatively, the <tt>JUMP TO A</tt> instruction <tt>JA</tt> can be used.
On entry to the subroutine, shift the value left by 8 bits (<tt>BLS 8</tt>)
and store it into a scratchpad register or other temporary memory location.
When the subroutine returns, reload the saved value into A and use
<tt>JA</tt> to jump back to the caller.  The following example uses
scratchpad register 7 as a link register:

        JM label
        ...
    label:
        BLS 8
        XC 7
        ...
        XC 7
        JA

None of these solutions support recursion, so the programmer will need to
make other arrangements to save A to a stack on entry to the subroutine, and
pop the value from the stack before returning.

## Pointers

The instruction set only supports absolute loads (CA) and stores (ST).
This can make it difficult to write code that involves pointers.
It is possible to get the effect of pointers using self-modifying code to
insert a pointer value into a preformatted CA or ST instruction.

For example, the following takes a pointer value in A, adds it to the
preformatted instruction at "load" and then executes it to load the
value at the pointer back into A:

        BLS 16          ; Shift the pointer in A up by 16 bits
        AD load         ; to overwrite the "CA 0" instruction below.
        ST 7            ; Save the new instruction in S7
        JU 7            ; and jump to it.
    load:
        CA 0
        JU load2
    load2:
        ...

Store can be done in a similar manner.  The following example stores S0
to the pointer value in A.  A is destroyed in the process:

        BLS 16          ; Shift the pointer in A up by 16 bits
        AD store        ; to overwrite the "ST 0" instruction below.
        ST 7            ; Save the new instruction in S7.
        CA 0            ; Load the value to store from S0 into A.
        JU 7            ; Jump to the new instruction.
    store:
        ST 0
        JU store2
    store2:
        ...

As an alternative, the load or store instruction could be built in A and
then executed using JA.  The <tt>hello\_world\_pointers</tt> example
uses this approach.
