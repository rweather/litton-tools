#!/usr/bin/python3
#
# Usage: convert-track.py input.csv
#
# Converts raw analog track data into digital 1's and 0's.

import sys
import csv

# Takes in a list of analog samples and digitizes them into 0 or 1.
# Each element in the list is assumed to be (Z1, Z2, Z3, track).
# It is also assumed that the samples are 100ns / 0.1us apart.
def digitize(samples):
    # Pick the cut-off points for determining if a signal is a 0 or a 1.
    Z1_low = 1.9            # Cut-off at 2V with 0.1V hysteresis.
    Z1_high = 2.0
    Z2_low = 1.9
    Z2_high = 2.0
    Z3_low = 1.9
    Z3_high = 2.0
    track_low = 1.0         # Strict 1V cut-off for the data track.
    track_high = 1.0

    # 100k or 200k samples?
    if len(samples) > 150000:
        samples_200k = True
    else:
        samples_200k = False

    # Digitize the samples with hysteresis.
    result = []
    Z1_prev = 0
    track_prev_prev = 0
    track_prev = 0
    T4_prev = 0
    T7_prev = 0
    Z1_current = 0
    Z2_current = 0
    Z3_current = 0
    track_current = 0
    T4_current = 0
    T7_current = 0
    T39_current = 0
    Z2_filtered = 0
    Z3_filtered = 0
    track_filtered = 0
    for Z1, Z2, Z3, track in samples:
        # Convert the samples into 0 or 1.
        if Z1 <= Z1_low:
            Z1_current = 0
        elif Z1 > Z1_high:
            Z1_current = 1
        if Z2 <= Z2_low:
            Z2_current = 0
        elif Z2 > Z2_high:
            Z2_current = 1
        if Z3 <= Z3_low:
            Z3_current = 0
        elif Z3 > Z3_high:
            Z3_current = 1
        if track <= track_low:
            track_current = 1   # Invert the track signal from the source.
        elif track > track_high:
            track_current = 0

        # Find the positive and negative edges on Z1 which is the main clock.
        if Z1_prev != 0 and Z1_current == 0:
            neg_edge = True
        else:
            neg_edge = False
        if Z1_prev == 0 and Z1_current != 0:
            pos_edge = True
        else:
            pos_edge = False
        Z1_prev = Z1_current

        # Latch Z2, Z3, and track on the negative-going edge of Z1.
        if neg_edge:
            Z2_filtered = Z2_current
            Z3_filtered = Z3_current
            # Insert a 100ns delay into the track to match the hardware.
            if samples_200k:
                track_filtered = track_prev_prev
            else:
                track_filtered = track_prev
        track_prev_prev = track_prev
        track_prev = track_current

        # Derive T4, T7, and T39.
        if neg_edge:
            T4_current = T4_prev
            T7_current = T7_prev
        if pos_edge:
            if T7_current != 0 and Z2_current == 0:
                T4_prev = 1
            if T7_current == 0:
                T4_prev = 0
            if T4_current == 0 and Z2_current != 0:
                T7_prev = 1
            if T4_current != 0 and Z2_current != 0:
                T7_prev = 0
        if T7_current == 0 and T4_current != 0:
            T39_current = 1
        else:
            T39_current = 0

        # Add the filtered values to the result.
        result.append((Z1_current, Z2_filtered, Z3_filtered, track_filtered, T4_current, T7_current, T39_current))
    return result

# Main entry point to the script.
def main():
    # Read the raw data from the CSV file.  The CSV input data is
    # assumed to be timestamp, Z1, Z2, Z3, track.
    raw_data = []
    with open(sys.argv[1]) as csvfile:
        for line in csv.reader(csvfile):
            try:
                Z1 = float(line[1])
                Z2 = float(line[2])
                Z3 = float(line[3])
                track = float(line[4])
                raw_data.append((Z1, Z2, Z3, track))
            except ValueError:
                # Skip the line if it is unparsable; probably a heading line.
                pass

    # Digitize the data into ones and zeroes.  Dump it in CSV format using the
    # same layout as Figure 3.4.1 on Page 3.12 of the technical manual.
    samples = digitize(raw_data)
    print("Index,Z1,Z2,T7,T4,T39,Z3,Track")
    posn = 0
    for Z1, Z2, Z3, track, T4, T7, T39 in samples:
        print("%d,%d,%d,%d,%d,%d,%d,%d" % (posn, Z1, Z2, T7, T4, T39, Z3, track))
        posn = posn + 1
        #if posn > 3000:
        #    break

if __name__ == "__main__":
    main()
