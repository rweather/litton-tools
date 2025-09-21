#!/usr/bin/python3
#
# Usage: extract-data.py input.csv track-number

import sys
import csv

# Find the first good T39 pulse.  The start of the input data may be
# distorted if the capture starts in the middle of a sector so we
# skip to the 4'th T39 pulse and start from there.  The signals should
# be stable by that point.
def find_good_T39(samples):
    posn = 0
    count = 0
    while posn < len(samples):
        if samples[posn][1] != 0:
            count = count + 1
            if count >= 4:
                return posn
            while (posn + 1) < len(samples) and samples[posn + 1][1] != 0:
                posn = posn + 1
        posn = posn + 1
    return None

# Find the next T39 pulse after this one.
def find_next_T39(samples, posn):
    while samples[posn][1] != 0:
        posn = posn + 1
    while samples[posn][1] == 0:
        posn = posn + 1
    return posn

# Find the location of the previous bit, which is just after the
# last falling edge of Z1.  Assumed to be positioned just after the
# falling edge of the current bit.
def find_prev_bit(samples, posn):
    posn = posn - 1
    while samples[posn][0] != 0:
        posn = posn - 1
    while samples[posn][0] == 0:
        posn = posn - 1
    return posn + 1

# Find the location of the next bit, which is just after the falling
# edge of the next Z1 pulse.
def find_next_bit(samples, posn):
    while samples[posn][0] == 0:
        posn = posn + 1
    while samples[posn][0] != 0:
        posn = posn + 1
    return posn

# Read a sector number just prior to a T39 pulse and turn it into an address.
# On entry, "posn" points to the first 1 on the rising edge of the T39 pulse.
def read_sector_address(samples, posn, track_number):
    sector = 0
    for bit in range(0, 7):
        # Sample +2 to get in the middle of bit time instead of on the edge.
        sector = sector * 2 + samples[posn + 2][2]
        posn = find_prev_bit(samples, posn)
    return int(sector + track_number * 128)

# Extract a sector word from the sample data.  On entry, "posn" points to the
# first 1 on the rising edge of the previous word's T39 pulse.
def read_sector_word(samples, posn):
    word = 0
    posn = find_next_bit(samples, posn)
    for bit in range(0, 40):
        posn = find_next_bit(samples, posn)
        # Sample +2 to get in the middle of bit time instead of on the edge.
        word = word + (samples[posn + 2][3] * (1 << bit))
    return int(word)

# Main entry point to the script.
def main():
    track_number = int(sys.argv[2])
    if track_number < 0 or track_number > 31:
        print("invalid track number", file=sys.stderr)
        sys.exit(1)

    # Read the digitized data from the CSV file.  The CSV input data is
    # assumed to be Index, Z1, Z2, T7, T4, T39, Z3, Track.
    samples = []
    with open(sys.argv[1]) as csvfile:
        for line in csv.reader(csvfile):
            try:
                Z1 = float(line[1])
                Z2 = float(line[2])
                T7 = float(line[3])
                T4 = float(line[4])
                T39 = float(line[5])
                Z3 = float(line[6])
                track = float(line[7])
                # We only care about Z1, T39, Z3, and track in this script.
                samples.append((Z1, T39, Z3, track))
            except ValueError:
                # Skip the line if it is unparsable; probably a heading line.
                pass

    # Find the first good sector to start extracting.
    posn = find_good_T39(samples)

    # Extract the words in the rest of the trace.  They may repeat.
    words = {}
    try:
        while posn < len(samples):
            address = read_sector_address(samples, posn, track_number)
            word = read_sector_word(samples, posn)
            if address in words:
                if words[address] != word:
                    print("mismatch at address %03X, was %010X, now %010X" % (address, word[address], word), file=sys.stderr)
            else:
                words[address] = word
            posn = find_next_T39(samples, posn)
    except:
        # Will throw an exception when we index off the end on a partial word.
        # Ignore the exception and continue with the next step.
        pass

    # Dump the words in address order and check for missing words.
    for address in range(track_number * 128, (track_number + 1) * 128):
        if address in words:
            print("%03X:%010X" % (address, words[address]))
        else:
            print("word for address %03X is missing" % address, file=sys.stderr)


if __name__ == "__main__":
    main()
