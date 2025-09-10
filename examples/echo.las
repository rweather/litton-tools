    title "Echo Test"
    printer $11,"EBS1231"
    keyboard $11,"EBS1231"
    org $F00
start:
    isw printer         ; Select the printer device for output.
    oiow "-"
    oiow "-"
    oiow "-"
    oiow " "
    oiow "T"
    oiow "y"
    oiow "p"
    oiow "e"
    oiow " "
    oiow "S"
    oiow "o"
    oiow "m"
    oiow "e"
    oiow "t"
    oiow "h"
    oiow "i"
    oiow "n"
    oiow "g"
    oiow " "
    oiow "-"
    oiow "-"
    oiow "-"
    oiow "\r"
    oiow "\n"
loop:
    isw keyboard        ; Select the keyboard device for input.
getkey:
    cio                 ; Input with odd parity.
    jc gotkey
    ju getkey
gotkey:
    isw printer         ; Select the printer device for output.
    bls 32
    oaow                ; Output A to the printer.
    cm
    ad const_CR
    tz
    jc crlf
    ju loop
crlf:
    ca const_LF         ; Print a LF after a CR.
    oaow
    ju loop
;
const_CR:
    db "\r"
const_LF:
    db "\n"
;
    org $FFF
boot:
    ju start
    entry boot
