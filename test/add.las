;
; Copyright (C) 2025 Rhys Weatherley
;
; Permission is hereby granted, free of charge, to any person obtaining a
; copy of this software and associated documentation files (the "Software"),
; to deal in the Software without restriction, including without limitation
; the rights to use, copy, modify, merge, publish, distribute, sublicense,
; and/or sell copies of the Software, and to permit persons to whom the
; Software is furnished to do so, subject to the following conditions:
;
; The above copyright notice and this permission notice shall be included
; in all copies or substantial portions of the Software.
;
; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
; OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
; AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
; FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
; DEALINGS IN THE SOFTWARE.
;
    title "Addition Tests"
    org $300
;
start:
;
; Test 1: A = 0 plus K = 0 equals A = 0
;
test_1:
    ca const_pi         ; Set A to non-zero.
    cl                  ; So that we can test if CL clears A to zero.
    rk
    ak
    tz
    jc test_2
    hh 1
;
; Test 2: A = 0 plus K = 1 equals A = 1
;
test_2:
    ca const_1
    st 0
    cl
    sk
    ak
    te 0
    jc test_3
    hh 1
;
; Test 3: 2 + 2 = 4
;
test_3:
    ca const_4
    st 1
    ca const_2
    ad const_2
    jc fail         ; No carry expected.
    te 1
    jc test_4
    hh 1
;
; Test 4: 2 + -5 = -3
;
test_4:
    ca const_neg_3
    st 0
    ca const_2
    ad const_neg_5
    jc fail         ; No carry expected.
    te 0
    jc test_5
    hh 1
;
; Test 5: -1 + 1 = 0 with carry
;
test_5:
    ca const_neg_1
    ad const_1
    jc test_5b      ; Carry expected.
    hh 1
test_5b:
    tz
    jc test_6
    hh 1
;
; Test 6: 314159265 + 628318530 = 942477795
;
test_6:
    ca const_3pi
    st 2
    ca const_pi
    ad const_2pi
    te 2
    jc test_7
    hh 1
;
; Test 7: -314159265 + 628318530 = 314159265
;
test_7:
    ca const_pi
    st 0
    cm              ; Negate pi
    ad const_2pi
    te 0
    jc test_8
    hh 1
;
; Test 8: Add conditional.
;
;   314159265 + Cond(628318530) = 942477795 if K
;   314159265 + Cond(628318530) = 314159265 if not K
;
test_8:
    ca const_pi
    sk              ; Set K
    ac const_2pi
    te 2
    jc test_8b
    hh 1
test_8b:
    ca const_pi
    rk              ; Reset K
    ac const_2pi
    te 0
    jc success
    hh 1
;
success:
    hh 0
;
; General failure exit.
;
fail:
    hh 1
;
; Constants.
;
const_1:
    dw 1
const_2:
    dw 2
const_4:
    dw 4
const_neg_3:
    dw -3
const_neg_5:
    dw -5
const_neg_1:
    dw -1
const_pi:
    dw 314159265
const_2pi:
    dw 628318530
const_3pi:
    dw 942477795

    entry start
