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

int litton_load_drum
    (litton_state_t *state, const char *filename, uint8_t *use_mask)
{
    FILE *file;
    char buffer[BUFSIZ];
    size_t len;
    unsigned long line;
    unsigned long printer_device = 0;
    litton_charset_t printer_charset = LITTON_CHARSET_EBS1231;
    unsigned long keyboard_device = 0;
    litton_charset_t keyboard_charset = LITTON_CHARSET_EBS1231;
    int ok = 1;
    if ((file = fopen(filename, "r")) == NULL) {
        perror(filename);
        return 0;
    }
    if (use_mask) {
        memset(use_mask, 0, LITTON_DRUM_MAX_SIZE);
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
            } else if (!strncmp(buffer, "#Printer-Character-Set: ", 24)) {
                if (!litton_charset_from_name
                        (&printer_charset, buffer + 24, strlen(buffer + 24))) {
                    fprintf(stderr, "%s:%lu: invalid printer character set\n",
                            filename, line);
                    ok = 0;
                } else {
                    state->printer_charset = printer_charset;
                }
            } else if (!strncmp(buffer, "#Printer-Device:", 16)) {
                printer_device = strtoul(buffer + 16, NULL, 16);
                if (!litton_is_valid_device_id(printer_device)) {
                    fprintf(stderr, "%s:%lu: invalid printer device identifier\n",
                            filename, line);
                    ok = 0;
                } else {
                    state->printer_id = (uint8_t)printer_device;
                }
            } else if (!strncmp(buffer, "#Keyboard-Character-Set: ", 25)) {
                if (!litton_charset_from_name
                        (&keyboard_charset, buffer + 25, strlen(buffer + 25))) {
                    fprintf(stderr, "%s:%lu: invalid keyboard character set\n",
                            filename, line);
                    ok = 0;
                } else {
                    state->keyboard_charset = keyboard_charset;
                }
            } else if (!strncmp(buffer, "#Keyboard-Device:", 17)) {
                keyboard_device = strtoul(buffer + 17, NULL, 16);
                if (!litton_is_valid_device_id(keyboard_device)) {
                    fprintf(stderr, "%s:%lu: invalid keyboard identifier\n",
                            filename, line);
                    ok = 0;
                } else {
                    state->keyboard_id = (uint8_t)keyboard_device;
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
                if (use_mask) {
                    use_mask[addr] = 1;
                }
            }
        }
    }
    fclose(file);
    return ok;
}

int litton_save_drum(litton_state_t *state, const char *filename)
{
    FILE *file;
    unsigned addr;
    file = fopen(filename, "w");
    if (!file) {
        perror(filename);
        return 0;
    }
    fprintf(file, "#Litton-Drum-Image\n");
    fprintf(file, "#Drum-Size: %d\n", (int)(state->drum_size));
    fprintf(file, "#Entry-Point: %03X\n", state->entry_point);
    fprintf(file, "#Printer-Character-Set: %s\n",
            litton_charset_to_name(state->printer_charset));
    if (state->printer_id != 0) {
        fprintf(file, "#Printer-Device: %02X\n", state->printer_id);
    }
    fprintf(file, "#Keyboard-Character-Set: %s\n",
            litton_charset_to_name(state->keyboard_charset));
    if (state->keyboard_id != 0) {
        fprintf(file, "#Keyboard-Device: %02X\n", state->keyboard_id);
    }
    for (addr = 0; addr < state->drum_size; ++addr) {
        fprintf(file, "%03X:%010LX\n", addr,
                (unsigned long long)(state->drum[addr]));
    }
    fclose(file);
    return 1;
}
