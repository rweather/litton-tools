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
#include "litton-opus.h"

static int litton_from_hex(int ch)
{
    if (ch >= 'A' && ch <= 'F') {
        return ch - 'A' + 10;
    } else if (ch >= 'a' && ch <= 'f') {
        return ch - 'a' + 10;
    } else {
        return ch - '0';
    }
}

static int litton_read_tape_word(FILE *file, litton_word_t *word)
{
    int ch;

    /* Zero the in-progress word */
    *word = 0;

    /* Skip whitespace before the next word */
    ch = getc(file);
    while (ch == ' ' || ch == '\r' || ch == '\n') {
        ch = getc(file);
    }

    /* Error if we have reached EOF or a terminator without a word */
    if (ch == EOF || !isxdigit(ch)) {
        return -2;
    }

    /* Read as many hexadecimal digits as we can */
    *word = litton_from_hex(ch);
    for (;;) {
        ch = getc(file);
        if (ch == EOF || !isxdigit(ch)) {
            break;
        }
        *word <<= 4;
        *word += litton_from_hex(ch);
    }
    return ch;
}

static int litton_load_tape
    (litton_state_t *state, const char *filename, uint8_t *use_mask, FILE *file)
{
    litton_word_t word = 0;
    litton_drum_loc_t addr = 0;
    int ok = 0;
    int ch;
    for (;;) {
        ch = litton_read_tape_word(file, &word);
        if (ch == -2) {
            ok = 0;
            break;
        } else if (ch == EOF || ch == ',') {
            /* Record the final word and stop */
            if (addr >= LITTON_DRUM_MAX_SIZE) {
                break;
            }
            word &= LITTON_WORD_MASK;
            litton_set_memory(state, addr, word);
            if (use_mask) {
                use_mask[addr] = 1;
            }
            ok = 1;
            break;
        } else if (ch == '/' || ch == '\r' || ch == '\n') {
            /* Record the current word and increment the address */
            if (addr >= LITTON_DRUM_MAX_SIZE) {
                break;
            }
            word &= LITTON_WORD_MASK;
            litton_set_memory(state, addr, word);
            if (use_mask) {
                use_mask[addr] = 1;
            }
            ++addr;
        } else if (ch == '#') {
            /* Address for a new range of words */
            if (word >= LITTON_DRUM_MAX_SIZE) {
                break;
            }
            addr = (litton_drum_loc_t)word;
        } else {
            /* Invalid terminator character */
            break;
        }
    }
    if (!ok) {
        fprintf(stderr, "%s: invalid tape image\n", filename);
    }
    return ok;
}

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
    int first_line = 1;
    if ((file = fopen(filename, "r")) == NULL) {
        perror(filename);
        return 0;
    }
    if (use_mask) {
        memset(use_mask, 0, LITTON_DRUM_MAX_SIZE);
    }
    line = 0;
    while (fgets(buffer, sizeof(buffer), file)) {
        /* If the first line starts with three hexadecimal digits and a '#'
         * then it is probably a Litton tape image instead of a drum image. */
        if (first_line && isxdigit(buffer[0]) && isxdigit(buffer[1]) &&
                isxdigit(buffer[2]) && buffer[3] == '#') {
            /* Seek back to the start of the file and read as a tape instead */
            if (fseek(file, 0, SEEK_SET) < 0) {
                perror(filename);
                fclose(file);
                return 0;
            }
            ok = litton_load_tape(state, filename, use_mask, file);
            fclose(file);
            return ok;
        }
        first_line = 0;

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
                litton_set_memory(state, addr, word);
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
                (unsigned long long)(litton_get_memory(state, addr)));
    }
    fclose(file);
    return 1;
}

void litton_load_opus(litton_state_t *state)
{
#if LITTON_SMALL_MEMORY
    /* OPUS is already implicitly loaded when using the small memory model */
    (void)state;
#else
    litton_drum_loc_t addr;
    for (addr = 0; addr < LITTON_DRUM_MAX_SIZE; ++addr) {
        litton_set_memory(state, addr, opus[addr]);
    }
#endif
}

#if LITTON_SMALL_MEMORY
const litton_word_t * const litton_opus = opus;
#endif
