;
; Calculate and print the Fibonacci sequence up to the limit of 40-bit numbers.
;
    title "Fibonacci"
    printer $41,"EBS1231"
    drumsize 2048
    org $700
start:
;
; The previous two elements of the sequence are in S0 and S1.
;
    cl              ; Set S0 to 0
    st 0
    sk              ; Set S1 to 1
    ak
    xc 1
;
; Main loop.
;
loop:
    ca 1            ; Copy S1 into S7
    st 7
    jm prnum        ; and print it in decimal
;
    xc 0            ; A = S0 + S1
    ad 1
    jc done         ; We are done if carry-out occurs
    xc 1            ; Swap A and S1
    xc 0            ; Copy the original S1 to S0
;
    ju loop
;
; Halt the machine at the end of the sequence.
;
done:
    hh 0
;
; Print the value in S7 in decimal, with leading zeroes.
;
prnum:
    xc 5            ; Save the return address in S5
;
    isw printer     ; Select the printer output device
;
; Algorithm:
;
;   for n = 11 to 1:
;       S6 = S7 / (10^n)
;       print S6 + '0'
;       S7 = S7 - S6 * (10^n)
;   print S7 + '0'
;
; This isn't super-efficient in code space but it is simple to implement.
;
    ca 7
    drs 11
    st 6
    tz
    jc digit1_zero
    ad const_digit_0
    bls 32
    oaw
    ca 6
    dls 11
    cm
    ad 7
    st 7
    ju digit2
digit1_zero:
    oiw "0"
    ca 7
;
digit2:
    drs 10
    st 6
    tz
    jc digit2_zero
    ad const_digit_0
    bls 32
    oaw
    ca 6
    dls 10
    cm
    ad 7
    st 7
    ju digit3
digit2_zero:
    oiw "0"
    ca 7
;
digit3:
    drs 9
    st 6
    tz
    jc digit3_zero
    ad const_digit_0
    bls 32
    oaw
    ca 6
    dls 9
    cm
    ad 7
    st 7
    ju digit4
digit3_zero:
    oiw "0"
    ca 7
;
digit4:
    drs 8
    st 6
    tz
    jc digit4_zero
    ad const_digit_0
    bls 32
    oaw
    ca 6
    dls 8
    cm
    ad 7
    st 7
    ju digit5
digit4_zero:
    oiw "0"
    ca 7
;
digit5:
    drs 7
    st 6
    tz
    jc digit5_zero
    ad const_digit_0
    bls 32
    oaw
    ca 6
    dls 7
    cm
    ad 7
    st 7
    ju digit6
digit5_zero:
    oiw "0"
    ca 7
;
digit6:
    drs 6
    st 6
    tz
    jc digit6_zero
    ad const_digit_0
    bls 32
    oaw
    ca 6
    dls 6
    cm
    ad 7
    st 7
    ju digit7
digit6_zero:
    oiw "0"
    ca 7
;
digit7:
    drs 5
    st 6
    tz
    jc digit7_zero
    ad const_digit_0
    bls 32
    oaw
    ca 6
    dls 5
    cm
    ad 7
    st 7
    ju digit8
digit7_zero:
    oiw "0"
    ca 7
;
digit8:
    drs 4
    st 6
    tz
    jc digit8_zero
    ad const_digit_0
    bls 32
    oaw
    ca 6
    dls 4
    cm
    ad 7
    st 7
    ju digit9
digit8_zero:
    oiw "0"
    ca 7
;
digit9:
    drs 3
    st 6
    tz
    jc digit9_zero
    ad const_digit_0
    bls 32
    oaw
    ca 6
    dls 3
    cm
    ad 7
    st 7
    ju digit10
digit9_zero:
    oiw "0"
    ca 7
;
digit10:
    drs 2
    st 6
    tz
    jc digit10_zero
    ad const_digit_0
    bls 32
    oaw
    ca 6
    dls 2
    cm
    ad 7
    st 7
    ju digit11
digit10_zero:
    oiw "0"
    ca 7
;
digit11:
    drs 1
    st 6
    tz
    jc digit11_zero
    ad const_digit_0
    bls 32
    oaw
    ca 6
    dls 1
    cm
    ad 7
    ju digit12
digit11_zero:
    oiw "0"
    ca 7
;
digit12:
    ad const_digit_0
    bls 32
    tz
    jc digit12_zero
    oaw
    ju digits_done
digit12_zero:
    oiw "0"
;
digits_done:
    oiw "\r"
    oiw "\n"
;
    ju 5            ; Jump to the return address in S5

;
; Constant pool.
;
const_digit_0:
    dw " "          ; In the EBS1231 character set, " " precedes "1", not "0".

    entry start
