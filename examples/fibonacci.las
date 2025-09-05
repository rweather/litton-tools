;
; Calculate and print the Fibonacci sequence up to the limit of 40-bit numbers.
;
    title "Fibonacci"
    printer $11,"ASCII"
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
    st 5            ; Save the return address in S5
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
    ad const_digit_0
    bls 32
    oaw
    ca 6
    dls 11
    cm
    ad 7
    st 7
;
    drs 10
    st 6
    ad const_digit_0
    bls 32
    oaw
    ca 6
    dls 10
    cm
    ad 7
    st 7
;
    drs 9
    st 6
    ad const_digit_0
    bls 32
    oaw
    ca 6
    dls 9
    cm
    ad 7
    st 7
;
    drs 8
    st 6
    ad const_digit_0
    bls 32
    oaw
    ca 6
    dls 8
    cm
    ad 7
    st 7
;
    drs 7
    st 6
    ad const_digit_0
    bls 32
    oaw
    ca 6
    dls 7
    cm
    ad 7
    st 7
;
    drs 6
    st 6
    ad const_digit_0
    bls 32
    oaw
    ca 6
    dls 6
    cm
    ad 7
    st 7
;
    drs 5
    st 6
    ad const_digit_0
    bls 32
    oaw
    ca 6
    dls 5
    cm
    ad 7
    st 7
;
    drs 4
    st 6
    ad const_digit_0
    bls 32
    oaw
    ca 6
    dls 4
    cm
    ad 7
    st 7
;
    drs 3
    st 6
    ad const_digit_0
    bls 32
    oaw
    ca 6
    dls 3
    cm
    ad 7
    st 7
;
    drs 2
    st 6
    ad const_digit_0
    bls 32
    oaw
    ca 6
    dls 2
    cm
    ad 7
    st 7
;
    drs 1
    st 6
    ad const_digit_0
    bls 32
    oaw
    ca 6
    dls 1
    cm
    ad 7
;
    ad const_digit_0
    bls 32
    oaw
;
    oiw "\r"
    oiw "\n"
;
    ju 5            ; Jump to the return address in S5

;
; Constant pool.
;
const_digit_0:
    dw "0"

    entry start
