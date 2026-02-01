#!/usr/bin/python3
#
# Usage: tape-to-punchable-py INPUT OUTPUT
#
# Reads in a tape in ASCII format and writes out a binary version that
# can be punched onto paper tape to feed into a Litton minicomputer.

import sys

# EBS1231 character set.  Numeric values in comments are in octal.
# "[x]" indicates a special function key.
# "{n}" indicates a printer command to move the print head to column n.
EBS1231 = [
    " ",        # 000
    "1",        # 001
    "2",        # 002
    "3",        # 003
    "4",        # 004
    "5",        # 005
    "6",        # 006
    "7",        # 007
    "8",        # 010
    "9",        # 011
    "@",        # 012
    "#",        # 013
    "[P1]",     # 014
    "[P2]",     # 015
    "[P3]",     # 016
    "[P4]",     # 017
    "0",        # 020
    "/",        # 021
    "S",        # 022
    "T",        # 023
    "U",        # 024
    "V",        # 025
    "W",        # 026
    "X",        # 027
    "Y",        # 030
    "Z",        # 031
    "*",        # 032
    ",",        # 033
    "[I]",      # 034
    "[II]",     # 035
    "[III]",    # 036
    "[IIII]",   # 037
    "-",        # 040
    "J",        # 041
    "K",        # 042
    "L",        # 043
    "M",        # 044
    "N",        # 045
    "O",        # 046
    "P",        # 047
    "Q",        # 050
    "R",        # 051
    "%",        # 052
    "$",        # 053
    "[LFB]",    # 054
    "[LFR]",    # 055
    "[BR]",     # 056
    "\f",       # 057
    "&",        # 060
    "A",        # 061
    "B",        # 062
    "C",        # 063
    "D",        # 064
    "E",        # 065
    "F",        # 066
    "G",        # 067
    "H",        # 070
    "I",        # 071
    "[072]",    # 072
    ".",        # 073
    "[RR]",     # 074
    "\n",       # 075
    "\b",       # 076
    "[TL]",     # 077
    "\r",       # 100
    "{4}",      # 101
    "{7}",      # 102
    "{10}",     # 103
    "{13}",     # 104
    "{16}",     # 105
    "{19}",     # 106
    "{22}",     # 107
    "{25}",     # 110
    "{28}",     # 111
    "{31}",     # 112
    "{34}",     # 113
    "{37}",     # 114
    "{40}",     # 115
    "{43}",     # 116
    "{46}",     # 117
    "{49}",     # 120
    "{52}",     # 121
    "{55}",     # 122
    "{58}",     # 123
    "{61}",     # 124
    "{64}",     # 125
    "{67}",     # 126
    "{70}",     # 127
    "{73}",     # 130
    "{76}",     # 131
    "{79}",     # 132
    "{82}",     # 133
    "{85}",     # 134
    "{88}",     # 135
    "{91}",     # 136
    "{94}",     # 137
    "{97}",     # 140
    "{100}",    # 141
    "{103}",    # 142
    "{106}",    # 143
    "{109}",    # 144
    "{112}",    # 145
    "{115}",    # 146
    "{118}",    # 147
    "{121}",    # 150
    "{124}",    # 151
    "{127}",    # 152
    "{130}",    # 153
    "{133}",    # 154
    "{136}",    # 155
    "{139}",    # 156
    "{142}",    # 157
    "{145}",    # 160
    "{148}",    # 161
    "{151}",    # 162
    "{154}",    # 163
    "{157}",    # 164
    "{160}",    # 165
    "{163}",    # 166
    "{166}",    # 167
    "{169}",    # 170
    "{172}",    # 171
    "{175}",    # 172
    "{178}",    # 173
    "{181}",    # 174
    "{184}",    # 175
    "{187}",    # 176
    "{190}"     # 177
]

# Convert an ASCII character sequence into an EBS1231 character.
def to_ebs1231(ch):
    global EBS1231
    try:
        return EBS1231.index(ch)
    except ValueError:
        # Encode 'X' if the ASCII character is unknown.
        return EBS1231.index('X')

# Add odd parity to a byte and then format it for punching.
def to_odd_parity(b):
    count = 0
    for bit in range(0, 8):
        if (b & (1 << bit)) != 0:
            count = count + 1
    result = (b & 0x0F) | ((b & 0x70) << 1);
    if (count & 1) == 0:
        result = result | 0x10
    return result

# Parse the command-line arguments.
if len(sys.argv) < 3:
    print('Usage: tape-to-punchable-py INPUT OUTPUT')
    sys.exit(1)

# Read the data from the input file.
data = []
with open(sys.argv[1], 'rb') as infile:
    bindata = infile.read()
    for b in bindata:
        data.append(int(b))

# Open the output file and write the converted data to it.
# Also add 40 zeroes to the start and end as tape leader and trailer.
with open(sys.argv[2], 'wb') as outfile:
    convdata = b''
    for i in range(0, 40):
        convdata += bytes([to_odd_parity(0)])
    ch = ''
    in_special = False
    for b in data:
        if in_special:
            ch += chr(b)
            if b == 0x5D or b == 0x7D:  # ] or } to end special
                in_special = False
            else:
                continue
        elif b == 0x5B or b == 0x7B:    # [ or { to start special
            ch = chr(b)
            in_special = True
            continue
        else:
            ch = chr(b)
        result = to_odd_parity(to_ebs1231(ch))
        convdata += bytes([result])
    for i in range(0, 40):
        convdata += bytes([to_odd_parity(0)])
    outfile.write(convdata)
