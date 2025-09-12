    title "HELLORLD!"
    printer $41,"EBS1231"
    drumsize 2048
    org $300
start:
    isw printer         ; Select the printer device for output.
    oiw "H"             ; Output the characters of the message.
    oiw "E"
    oiw "L"
    oiw "L"
    oiw "O"
    oiw "R"
    oiw "L"
    oiw "D"
    oiw "\r"
    oiw "\n"
    hh 0                ; Halt.
    entry start
