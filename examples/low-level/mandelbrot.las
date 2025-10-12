;
; Mandelbrot implementation for the Litton computer in low-level machine code.
;
; This implementation uses 16-bit fixed-point values with 6 bits before the
; decimal point and 10 bits after the decimal point.
;
; Implementation ideas originally from:
; https://github.com/ttsiodras/Blue_Pill_Mandelbrot/blob/master/BluePill_TFT16K_Mandelbrot.ino
;

    title "Mandelbrot"
    printer $41,"EBS1231"
    drumsize 4096
    org $800
start:
    ju start2

;
; Storage for local variables.
;
a:
    dw 0
b:
    dw 0
c:
    dw 0
d:
    dw 0
e:
    dw 0
r:
    dw 0
i:
    dw 0
x:
    dw 0

;
; Prototype pointer lookup routine for loading the value at chars+N.
; This is positioned at $809 so that the high byte ends up as $0A.
; The $0A / no-op makes the whole word suitable for passing to "JA".
;
    org $809
load_char:
    ca chars
    ju got_char

;
; More storage for local variables.
;
y:
    dw 0

start2:

;
; Select the printer device.
;
    isw printer

;
; Put the value just above 4 into S6 for use in "TG" comparisons later.
;
    ca const_four_gt
    xc 6

;
; Set the initial x and y positions.
;
    ca const_x1
    st x
    ca const_y1
    st y

;
; Start processing the next point.  Set r = i = 0, k = max_iterations.
;
next_point:
    cl
    st r
    st i
    ca const_max_iterations
    xc 5                ; k is in S5.

;
; One iteration of the Mandelbrot calculation:
;
;   Input: r, i, x, y
;   a = r^2
;   b = i^2
;   c = (r - i)^2
;   d = a - b
;   e = a + b
;   r = d + x
;   i = e - c + y
;   stop if e > 4
;
iteration:
    ca r                ; a = r^2
    xc 0
    jm square
    st a
;
    ca i                ; b = i^2
    xc 0
    jm square
    st b
;
    cm                  ; d = a - b
    ad a
    st d
;
    ca i                ; c = (r - i)^2
    cm
    ad r
    xc 0
    jm square
    st c
;
    ca a                ; e = a + b
    ad b
    st e
;
    ca d                ; r = d + x
    ad x
    st r
;
    ca c                ; i = e - c + y
    cm
    ad e
    ad y
    st i
;
    ca e                ; Stop if e > 4.
    tg 6
    jc end_iterations
    xc 5                ; Decrement k and stop when we get to zero.
    ad const_neg_one
    st 5
    tz
    jc end_iterations
    ju iteration        ; Go back for another iteration.

;
; End of the Mandelbrot iterations.  S5 is the counter where we bailed out.
; Print a character based on the counter.
;
end_iterations:
    xc 5
    bls 16
    ad load_char
    ja
got_char:
    bls 32
    oao                 ; Output the character.

;
; Advance to the next point and check for end of line / end of program.
;
    ca const_x2
    xc 0
    ca x
    ad const_xstep
    st x
    th
    jc next_point       ; Still on the current line if x is negative.
    tg 0
    jc next_line
    ju next_point
next_line:
    oiw "\r"            ; Print CRLF at the end of each line.
    oiw "\n"
    ca const_x1
    st x
    ca const_y2
    xc 0
    ca y
    ad const_ystep
    st y
    th
    jc next_point       ; Still more to go if y is negative.
    tg 0
    jc finished
    ju next_point
finished:
    hh 0                ; Halt.

;
; Square the value in S0, result in A.
;
; Destroys S0, S1, S2, S3, S4, S7
;
square:
    xc 7                ; Put the return address in S7.
    ca const_neg_16
    st 4                ; S4 is the loop counter, increments up to zero.
    cl
    xc 0                ; Load S0 into A and zero S0.
    th                  ; Get the absolute value of the argument.
    jc square_neg
    ju square_pos
square_neg:
    cm
square_pos:
    st 2                ; Store the multiplicand into S2.
    bls 24              ; Shift the MSB of the multiplier up by 24 bits.
    xc 1                ; Put the multiplier in the high bits of S1.
square_loop:
    bld 1               ; Shift the answer in S0/S1 up by 1 bit.
    jc square_add_carry ; If the high bit was set, we need to add S2 to S0/S1.
square_next:
    ca 4                ; Increment the loop counter.
    sk
    ak
    st 4
    jc square_done      ; Bail out if the loop counter is now zero.
    ju square_loop      ; Otherwise back around for more.
square_add_carry:
    ca 2                ; Add S2 to S0/S1.
    ad 0
    st 0
    ju square_next
square_done:
;
; Shift S0/S1 down to reposition the decimal point.
;
    brd 10
    xc 0                ; Shift the result into A.
    ju 7                ; Return from the subroutine.

;
; Constants.
;
const_max_iterations:
    dw 20
const_neg_16:
    dw -16
const_neg_one:
    dw -1
const_four_gt:
    dw $0000001001      ; Just a bit over 4.
const_x1:
    dw $FFFFFFF800      ; -2.00
const_x2:
    dw $0000000400      ; +1.00
const_xstep:
    dw $0000000030      ; (2.00 + 1.00) / 64
const_y1:
    dw $FFFFFFFC00      ; -1.00
const_y2:
    dw $0000000400      ; +1.00
const_ystep:
    dw $0000000055      ; (1.00 + 1.00) / 24

;
; Characters to print for the different numbers of iterations that
; occurred before the point jumped out of the Mandelbrot set.
; There are enough characters here for up to 20 iterations.
;
chars:
    dw " .,-*/$&#@0123456789A"

    entry start
