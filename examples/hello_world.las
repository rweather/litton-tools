    title "Hello, World!"
    printer $41,"EBS1231"
    drumsize 4096
    org $800
start:
    isw printer         ; Select the printer device for output.
    oiw "H"             ; Output the characters of the message.
    oiw "e"
    oiw "l"
    oiw "l"
    oiw "o"
    oiw ","
    oiw " "
    oiw "W"
    oiw "o"
    oiw "r"
    oiw "l"
    oiw "d"
    oiw "\r"
    oiw "\n"
    hh 0                ; Halt.
    entry start
