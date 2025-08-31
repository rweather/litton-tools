/*
 * Copyright (C) 2025 Rhys Weatherley
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <litton/litton.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int litton_load_drum(litton_state_t *state, const char *filename)
{
    FILE *file;
    char buffer[BUFSIZ];
    size_t len;
    unsigned long line;
    unsigned long console_device = 0;
    int ok = 1;
    if ((file = fopen(filename, "r")) == NULL) {
        perror(filename);
        return 0;
    }
    line = 0;
    while (fgets(buffer, sizeof(buffer), file)) {
        /* Trim white space from the end of the line */
        ++line;
        len = strlen(buffer);
        while (len > 0 && isspace(buffer[len - 1])) {
            --len;
        }
        buffer[len] = '\0';

        /* Metadata or data? */
        if (buffer[0] == '#') {
            if (!strncmp(buffer, "#Drum-Size:", 11)) {
                litton_set_drum_size
                    (state, strtoul(buffer + 11, NULL, 0));
            } else if (!strncmp(buffer, "#Entry-Point:", 13)) {
                litton_set_entry_point
                    (state, strtoul(buffer + 13, NULL, 16));
            } else if (!strncmp(buffer, "#Console-Character-Set:", 23)) {
                // TODO
            } else if (!strncmp(buffer, "#Console-Device:", 16)) {
                console_device = strtoul(buffer + 16, NULL, 16) & 0xFF;
                if ((console_device & 0xF0) == 0 ||
                        (console_device & 0x0F) == 0) {
                    fprintf(stderr, "%s:%lu: invalid device identifier\n",
                            filename, line);
                    ok = 0;
                }
            }
        } else if (buffer[0] != '\0') {
            unsigned long addr = 0;
            unsigned long long word = 0;
            if (sscanf(buffer, "%lx:%Lx", &addr, &word) != 2) {
                fprintf(stderr, "%s:%lu: invalid drum data '%s'\n",
                        filename, line, buffer);
                ok = 0;
            } else {
                /* Clamp the address and word into range, and store */
                addr &= LITTON_DRUM_MAX_SIZE - 1;
                word &= LITTON_WORD_MASK;
                state->drum[addr] = word;
            }
        }
    }
    fclose(file);
    if (ok && console_device != 0) {
        /* Set up the console for the machine as described in the drum image */
        // TODO: character set.
        litton_add_console(state, console_device, LITTON_CHARSET_ASCII);
    }
    return ok;
}
