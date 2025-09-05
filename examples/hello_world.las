    title "Hello, World!"
    printer $11,"ASCII"
    drumsize 2048
    org $300
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
    oiw "!"
    oiw "\r"
    oiw "\n"
    hh 0                ; Halt.
    entry start
