#!/usr/bin/python3
#
# Usage: convert-tape-py [binary|hex] INPUT OUTPUT
#
# Use "binary" or "hex" to describe the format of the INPUT file.
# OUTPUT is in ASCII after conversion from the EBS1232 character set.

import sys
import re

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

# Check if a byte has odd parity.
def is_odd_parity(b):
    count = 0
    for bit in range(0, 8):
        if (b & (1 << bit)) != 0:
            count = count + 1
    return (count & 1) != 0

# Parse the command-line arguments.
if len(sys.argv) < 4:
    print('Usage: convert-tape-py [binary|hex] INPUT OUTPUT')
    sys.exit(1)
is_binary = False
is_hex = False
if sys.argv[1] == 'binary':
    is_binary = True
elif sys.argv[1] == 'hex':
    is_hex = True
else:
    print('Specify input format, either "binary" or "hex"')
    sys.exit(1)

# Read the data from the input file.
data = []
if is_binary:
    with open(sys.argv[2], 'rb') as infile:
        bindata = infile.read()
        for b in bindata:
            data.append(b)
else:
    with open(sys.argv[2], 'r') as infile:
        hexdata = re.sub(r'[^0-9A-Fa-f]', '', infile.read())
        posn = 0
        while posn < len(hexdata):
            data.append(int(hexdata[posn:posn+2], 16))
            posn = posn + 2

# Open the output file and write the converted data to it.
with open(sys.argv[3], 'wb') as outfile:
    convdata = ''
    for b in data:
        if not is_odd_parity(b):
            print('Input byte 0x%02x does not have odd parity' % b)
        ch = ((b & 0xE0) >> 1) | (b & 0x0F)
        convdata += EBS1231[ch]
    outfile.write(convdata.strip().encode('utf-8'))
