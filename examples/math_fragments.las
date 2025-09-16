;
; This file contains some fragments of routines to perform various
; mathematical operations.  Experimental to explore the instruction set.
;
; Arguments and results passed in S0, S1, S2, S3.  Link registers for
; subroutine return addresses in S6 and S7.  S2, S3, S4, S5 can be freely
; destroyed for temporary results.
;
    title "Math Fragments"
    printer $41,"EBS1231"
    org $300

;
; 80-bit addition of S2/S3 to S0/S1.
;
; Modifies S0, S1
; Destroys S7 (link)
;
add80:
    xc 7                ; Save the return address.
;
    ca 1                ; Low word.
    ad 3
    st 1
    ca 0                ; High word.
    ak                  ; Add the carry from the low word.
    ad 2
    st 0
;
    ju 7                ; Return from the subroutine.

;
; 40-bit AND of S2 with S0, result in S0.
;
; Modifies S0
; Destroys S7 (link)
;
and40:
    xc 7
    ca 2
    la 0
    st 0
    ju 7

;
; 40-bit NOT of S0.
;
; Modifies S0
; Destroys S7 (link)
;
not40:
    xc 7
    xc 0
    cm
    ad const_neg_1
    xc 0
    ju 7
const_neg_1:
    dw -1

;
; 40-bit OR of S2 with S0, result in S0.
;
; Modifies S0
; Destroys S7 (link)
;
; Uses DeMorgan's Law: A or B = not ((not A) and (not B))
;
or40:
    xc 7
or40_2:
    xc 0                ; Bitwise NOT on S0.
    cm
    ad const_neg_1
    st 0
    ca 2                ; Bitwise NOT on S2.
    cm
    ad const_neg_1
    la 0                ; And S0 and S2.
    cm
    ad const_neg_1      ; Bitwise NOT on the result.
    st 0
    ju 7

;
; 40-bit XOR of S2 with S0, result in S0.
;
; Modifies S0
; Destroys S2, S3, S7 (link)
;
; Algorithm: A xor B = (A and B) or ((not A) and (not B))
;
xor40:
    xc 7
;
    ca 0                ; S3 = S0 and S2.
    la 2
;
    xc 0                ; Swap AND result into S0 and bitwise NOT original S0.
    cm
    ad const_neg_1
    st 3                ; S3 = not S0.
;
    xc 2                ; Bitwise NOT on S2.
    cm
    ad const_neg_1
    la 3
    st 2                ; S2 = S3 and (not S2).
;
    ju or40_2           ; OR the sub-terms and return.

;
; 80-bit negation of S0/S1.
;
; Modifies S0, S1
; Destroys S7 (link)
;
neg80:
    xc 7
    xc 0
    cm
    ad const_neg_1
    xc 0
    xc 1
    cm
    ad const_neg_1
    xc 1
    xc 0
    ak
    xc 0
    xc 1
    sk
    ak
    xc 1
    xc 0
    ak
    xc 1
    ju 7

;
; 80-bit subtraction of S0/S1 from S2/S3, result in S0/S1.
;
; Modifies S0, S1
; Destroys S6 (link), S7
;
sub80:
    xc 6                ; Save the return address.
    jm neg80
    jm add80
    ju 6                ; Return from the subroutine.

;
; Signed multiplication of S2 and S3, giving a double word result in S0/S1.
;
; Modifies S0, S1
; Destroys S2, S3, S4, S6 (link), S7
;
smul:
    xc 6                ; Save the return address.
    ca 2
    th
    jc smul_s2_neg
    ca 3
    th
    jc smul_s3_neg
    jm mul              ; S2 and S3 are both positive.
smul_done:
    ju 6                ; Return from the subroutine.
smul_s2_neg:
    cm
    st 2
    ca 3
    th
    jc smul_s2_s3_neg
    jm mul              ; S2 is negative, S3 is positive.
    ju smul_neg
smul_s3_neg:
    cm
    st 3
    jm mul              ; S2 is positive, S3 is negative.
smul_neg:
    jm neg80            ; Negate the 80-bit result.
    ju smul_done
smul_s2_s3_neg:
    cm
    st 3
    jm mul              ; S2 and S3 are both negative.
    ju smul_done

;
; Unsigned multiplication of S2 and S3, giving a double word result in S0/S1.
;
; Modifies S0, S1
; Destroys S2, S4, S7 (link)
;
mul:
    xc 7                ; Save the return address.
;
    cl                  ; Clear the result in S0/S1 to zero.
    st 0
    st 1
    ca const_neg_40     ; Negative loop counter in S4.
    st 4
    ju mul_loop
;
mul_add:
    ca 3                ; Add S3 to S0/S1.
    ad 1
    st 1
    ca 0
    ak
    st 0
;
mul_next:
    xc 4                ; Increment the loop counter and check if zero.
    sk
    ak
    tz
    xc 4
    jc mul_end
;
mul_loop:
    bld 1               ; Multiply S0/S1 by 2.
    ca 2                ; Shift S2 left by one bit.
    bls 1
    st 2
    jc mul_add          ; Add S3 to S0/S1 if the shifted-out bit is 1.
    ju mul_next         ; No add.
;
mul_end:
    ju 7                ; Return from the subroutine.
;
const_neg_40:
    dw  -40

;
; Unsigned division of S0/S1 by S2 giving a quotient in S0/S1 and a
; remainder in S3.
;
; Modifies S0, S1, S3
; Destroys S1, S4, S7 (link)
;
div:
    xc 7                ; Save the return address.
;
    cl                  ; Set the remainder word in S3 to zero.
    st 3
;
    ca const_neg_80     ; Negative loop counter in S4.
    st 4
    ju div_loop
;
div_1:
    st 3                ; S3 >= S2, so store the subtraction result.
div_0:
    ca 1                ; Shift the quotient bit into place in the
    ak                  ; lowest bit of S0/S1.
    st 1
;
    xc 4                ; Increment the loop counter and check if zero.
    sk
    ak
    tz
    xc 4
    jc div_end
;
div_loop:
    bld 1               ; Multiply S0/S1 by 2 and shift a bit into S3.
    ca 3
    blsk 1
    st 3
    ca 2
    cm
    ad 0                ; A = S3 - S2, sets carry if S3 >= S2.
    jc div_1
    ju div_0
;
div_end:
    ju 7                ; Return from the subroutine.
const_neg_80:
    dw  -80

start:
    hh 0                ; Halts and does nothing.
    entry start
