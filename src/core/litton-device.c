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

void litton_add_device(litton_state_t *state, litton_device_t *device)
{
    device->selected = 0;
    device->next = state->devices;
    state->devices = device;
}

static void litton_deselect_device
    (litton_state_t *state, litton_device_t *device)
{
    if (device != 0) {
        if (device->selected && device->deselect != 0) {
            (*(device->deselect))(state, device);
        }
        device->selected = 0;
    }
}

static int litton_device_match
    (const litton_device_t *device, int device_select_code)
{
    return device->id != 0 && (device_select_code & device->id) == device->id;
}

int litton_select_device(litton_state_t *state, int device_select_code)
{
    litton_device_t *device = state->devices;
    while (device != 0) {
        if (litton_device_match(device, device_select_code)) {
            /* If the device is not currently selected, then select it */
            if (!device->selected) {
                if (device->select != 0) {
                    (*(device->select))(state, device);
                }
                device->selected = 1;
            }
        } else if (device->selected) {
            /* Device was selected, but it is not anymore */
            litton_deselect_device(state, device);
        }
        device = device->next;
    }
    return 0;
}

int litton_is_output_busy(litton_state_t *state)
{
    litton_device_t *device = state->devices;
    while (device != 0) {
        if (device->selected && device->supports_output) {
            if (device->is_busy != 0 && (*(device->is_busy))(state, device)) {
                return 1;
            }
        }
        device = device->next;
    }
    return 0;
}

void litton_output_to_device
    (litton_state_t *state, uint8_t value, litton_parity_t parity)
{
    litton_device_t *device = state->devices;
    while (device != 0) {
        if (device->selected && device->supports_output) {
            if (!(device->is_busy) || !((*device->is_busy))(state, device)) {
                if (device->output != 0) {
                    (*(device->output))(state, device, value, parity);
                }
            }
        }
        device = device->next;
    }
}

int litton_input_from_device
    (litton_state_t *state, uint8_t *value, litton_parity_t parity)
{
    litton_device_t *device = state->devices;
    while (device != 0) {
        if (device->selected && device->supports_input) {
            if (device->input != 0) {
                if ((*(device->input))(state, device, value, parity)) {
                    return 1;
                }
            }
        }
        device = device->next;
    }
    return 0;
}

int litton_input_device_status(litton_state_t *state, uint8_t *status)
{
    litton_device_t *device = state->devices;
    while (device != 0) {
        if (device->selected && device->supports_input) {
            if (device->status != 0) {
                if ((*(device->status))(state, device, status)) {
                    return 1;
                }
            }
        }
        device = device->next;
    }
    return 0;
}

static int litton_count_bits(uint8_t value)
{
    int count = 0;
    int bit;
    for (bit = 0; bit < 7; ++bit) {
        if ((value & (1 << bit)) != 0) {
            ++count;
        }
    }
    return count;
}

uint8_t litton_add_parity(uint8_t value, litton_parity_t parity)
{
    /*
     * Litton 1600 Technical Reference Manual, section 3.7, "Commands"
     *
     * The description of the commands imply that the least significant
     * bit is used for parity, but this is the reverse of normal practice
     * where the most significiant is used.
     *
     * Technically, RS232 sends the parity bit last, so it may have been
     * misinterpeted as the least significant bit by the manual writers.
     *
     * For now, assume that the most significant bit is the parity bit
     * to ease integration with standard systems.  Fix later if we have to.
     */
    if (parity == LITTON_PARITY_ODD) {
        if ((litton_count_bits(value) & 1) == 0) {
            value |= 0x80;
        } else {
            value &= 0x7F;
        }
    } else if (parity == LITTON_PARITY_EVEN) {
        if ((litton_count_bits(value) & 1) != 0) {
            value |= 0x80;
        } else {
            value &= 0x7F;
        }
    }
    return value;
}

uint8_t litton_remove_parity(uint8_t value, litton_parity_t parity)
{
    if (parity == LITTON_PARITY_NONE) {
        return value;
    } else {
        return value & 0x7F;
    }
}

static void litton_printer_output
    (litton_state_t *state, litton_device_t *device,
     uint8_t value, litton_parity_t parity)
{
    (void)state;
    if (device->charset != LITTON_CHARSET_HEX) {
        value = litton_remove_parity(value, parity);
    }
    if (device->charset == LITTON_CHARSET_EBS1231) {
        /* Does this look like a print wheel position? */
        uint8_t position = litton_print_wheel_position(value);
        if (position != 0) {
            /* Yes, so space forward or backspace back to put the
             * print head in the right column. */
            --position;
            while (device->print_position < position) {
                putc(' ', stdout);
                ++(device->print_position);
            }
            while (device->print_position > position) {
                putc('\b', stdout);
                --(device->print_position);
            }
        } else if (value == 075 || value == 055 || value == 054) {
            /* Line Feed Left / Line Feed Right / Line Feed Both */
            putc('\n', stdout);
        } else {
            /* Convert the code into its ASCII form */
            const char *string_form;
            int ch = litton_char_from_charset
                (value, device->charset, &string_form);
            if (ch == '\n' || ch == '\f') {
                /* Output a carriage return and line feed */
                putc('\r', stdout);
                putc('\n', stdout);
                device->print_position = 0;
            } else if (ch == '\r') {
                putc(ch, stdout);
                device->print_position = 0;
            } else if (ch == '\b') {
                putc('\b', stdout);
                if (device->print_position > 0) {
                    --(device->print_position);
                }
            } else if (ch >= 0) {
                /* Single character */
                putc(ch, stdout);
            } else if (ch == -2) {
                /* Multi-character string */
                fputs(string_form, stdout);
                device->print_position += strlen(string_form);
            }
        }
    } else if (device->charset == LITTON_CHARSET_HEX) {
        /* Output the bytes in hexadecimal */
        if (device->print_position > 0) {
            putc(' ', stdout);
        }
        printf("%02X", value);
        ++(device->print_position);
        if (device->print_position >= 16) {
            printf("\n");
            device->print_position = 0;
        }
    } else {
        /* Assume plain ASCII codes as input */
        putc(value, stdout);
    }
    fflush(stdout);
}

void litton_create_default_devices(litton_state_t *state)
{
    if (state->printer_id != 0) {
        litton_add_printer(state, state->printer_id, state->printer_charset);
    }
    if (state->keyboard_id != 0) {
        litton_add_keyboard(state, state->keyboard_id, state->keyboard_charset);
    }
}

void litton_add_printer
    (litton_state_t *state, uint8_t id, litton_charset_t charset)
{
    litton_device_t *device = calloc(1, sizeof(litton_device_t));
    device->id = id;
    device->supports_input = 0;
    device->supports_output = 1;
    device->charset = charset;
    device->output = litton_printer_output;
    litton_add_device(state, device);
}

static int litton_keyboard_input
    (litton_state_t *state, litton_device_t *device,
     uint8_t *value, litton_parity_t parity)
{
    // TODO
    (void)state;
    (void)device;
    (void)value;
    (void)parity;
    return 0; /* Keyboard input is not ready */
}

void litton_add_keyboard
    (litton_state_t *state, uint8_t id, litton_charset_t charset)
{
    litton_device_t *device = calloc(1, sizeof(litton_device_t));
    device->id = id;
    device->supports_input = 1;
    device->supports_output = 0;
    device->charset = charset;
    device->input = litton_keyboard_input;
    litton_add_device(state, device);
}

void litton_add_input_tape
    (litton_state_t *state, uint8_t id, litton_charset_t charset,
     const char *filename)
{
    // TODO
    (void)state;
    (void)id;
    (void)charset;
    (void)filename;
}

void litton_add_output_tape
    (litton_state_t *state, uint8_t id, litton_charset_t charset,
     const char *filename)
{
    // TODO
    (void)state;
    (void)id;
    (void)charset;
    (void)filename;
}

/** Mapping table from Appendix V of the EBS/1231 System Programming Manual */
static const char * const litton_EBS1231_to_ASCII[128] = {
    /* Octal Code       ASCII */
    /* 000 */           " ",
    /* 001 */           "1",
    /* 002 */           "2",
    /* 003 */           "3",
    /* 004 */           "4",
    /* 005 */           "5",
    /* 006 */           "6",
    /* 007 */           "7",
    /* 010 */           "8",
    /* 011 */           "9",
    /* 012 */           "@",            /* Also the CLEAR key */
    /* 013 */           "#",            /* Also the P0 key */
    /* 014 */           "[P1]",
    /* 015 */           "[P2]",
    /* 016 */           "[P3]",
    /* 017 */           "[P4]",
    /* 020 */           "0",
    /* 021 */           "/",
    /* 022 */           "S",
    /* 023 */           "T",
    /* 024 */           "U",
    /* 025 */           "V",
    /* 026 */           "W",
    /* 027 */           "X",
    /* 030 */           "Y",
    /* 031 */           "Z",
    /* 032 */           "*",
    /* 033 */           ",",
    /* 034 */           "[I]",
    /* 035 */           "[II]",
    /* 036 */           "[III]",
    /* 037 */           "[IIII]",
    /* 040 */           "-",            /* Also the diamond key */
    /* 041 */           "J",
    /* 042 */           "K",
    /* 043 */           "L",
    /* 044 */           "M",
    /* 045 */           "N",
    /* 046 */           "O",
    /* 047 */           "P",
    /* 050 */           "Q",
    /* 051 */           "R",
    /* 052 */           "%",
    /* 053 */           "$",
    /* 054 */           "[LFB]",        /* Line feed both */
    /* 055 */           "[LFR]",        /* Line feed right */
    /* 056 */           "[BR]",         /* Black ribbon print */
    /* 057 */           "\f",           /* Form up */
    /* 060 */           "&",
    /* 061 */           "A",
    /* 062 */           "B",
    /* 063 */           "C",
    /* 064 */           "D",
    /* 065 */           "E",
    /* 066 */           "F",
    /* 067 */           "G",
    /* 070 */           "H",
    /* 071 */           "I",
    /* 072 */           "[072]",        /* Not used */
    /* 073 */           ".",
    /* 074 */           "[RR]",         /* Red ribbon print */
    /* 075 */           "\n",           /* Line feed left */
    /* 076 */           "\b",           /* Backspace */
    /* 077 */           "[TL]",         /* Carriage Open or Close / Tape Leader */
    /* 100 */           "\r",           /* Return printer to position 1 */
    /* 101 */           "{4}",          /* Printer wheel positions */
    /* 102 */           "{7}",
    /* 103 */           "{10}",
    /* 104 */           "{13}",
    /* 105 */           "{16}",
    /* 106 */           "{19}",
    /* 107 */           "{22}",
    /* 110 */           "{25}",
    /* 111 */           "{28}",
    /* 112 */           "{31}",
    /* 113 */           "{34}",
    /* 114 */           "{37}",
    /* 115 */           "{40}",
    /* 116 */           "{43}",
    /* 117 */           "{46}",
    /* 120 */           "{49}",
    /* 121 */           "{52}",
    /* 122 */           "{55}",
    /* 123 */           "{58}",
    /* 124 */           "{61}",
    /* 125 */           "{64}",
    /* 126 */           "{67}",
    /* 127 */           "{70}",
    /* 130 */           "{73}",
    /* 131 */           "{76}",
    /* 132 */           "{79}",
    /* 133 */           "{82}",
    /* 134 */           "{85}",
    /* 135 */           "{88}",
    /* 136 */           "{91}",
    /* 137 */           "{94}",
    /* 140 */           "{97}",
    /* 141 */           "{100}",
    /* 142 */           "{103}",
    /* 143 */           "{106}",
    /* 144 */           "{109}",
    /* 145 */           "{112}",
    /* 146 */           "{115}",
    /* 147 */           "{118}",
    /* 150 */           "{121}",
    /* 151 */           "{124}",
    /* 152 */           "{127}",
    /* 153 */           "{130}",
    /* 154 */           "{133}",
    /* 155 */           "{136}",
    /* 156 */           "{139}",
    /* 157 */           "{142}",
    /* 160 */           "{145}",
    /* 161 */           "{148}",
    /* 162 */           "{151}",
    /* 163 */           "{154}",
    /* 164 */           "{157}",
    /* 165 */           "{160}",
    /* 166 */           "{163}",
    /* 167 */           "{166}",
    /* 170 */           "{169}",
    /* 171 */           "{172}",
    /* 172 */           "{175}",
    /* 173 */           "{178}",
    /* 174 */           "{181}",
    /* 175 */           "{184}",
    /* 176 */           "{187}",
    /* 177 */           "{190}"
};

static int litton_ebs1231_match
    (const char *str, size_t *posn, size_t len, const char *sequence)
{
    size_t seq_len = strlen(sequence);
    if ((*posn + seq_len) > len) {
        return 0;
    }
    if (litton_name_match(sequence, str + *posn, seq_len)) {
        *posn += seq_len;
        return 1;
    }
    return 0;
}

int litton_char_to_charset
    (const char *str, size_t *posn, size_t len, litton_charset_t charset)
{
    int ch;
    if ((*posn) >= len) {
        return -1;
    }
    switch (charset) {
    case LITTON_CHARSET_ASCII:
        /* Plain ASCII, one character at a time */
        ch = str[(*posn)++];
        return ch & 0xFF;

    case LITTON_CHARSET_UASCII:
        /* Plain ASCII, but force the character to uppercase */
        ch = str[(*posn)++];
        if (ch >= 'a' && ch <= 'z') {
            ch = ch - 'a' + 'A';
        }
        return ch & 0xFF;

    case LITTON_CHARSET_EBS1231:
    case LITTON_CHARSET_HEX: /* Not supported for input at the moment */
        /* Scan the EBS1231 mapping table to find a match */
        for (ch = 0; ch < 128; ++ch) {
            if (litton_ebs1231_match
                    (str, posn, len, litton_EBS1231_to_ASCII[ch])) {
                return ch;
            }
        }
        ch = -1; /* Not found */
        break;
    }
    return -1;
}

int litton_char_from_charset
    (int ch, litton_charset_t charset, const char **string_form)
{
    static const char * const hex_bytes[] = {
        "00", "01", "02", "03", "04", "05", "06", "07",
        "08", "09", "0A", "0B", "0C", "0D", "0E", "0F",
        "10", "11", "12", "13", "14", "15", "16", "17",
        "18", "19", "1A", "1B", "1C", "1D", "1E", "1F",
        "20", "21", "22", "23", "24", "25", "26", "27",
        "28", "29", "2A", "2B", "2C", "2D", "2E", "2F",
        "30", "31", "32", "33", "34", "35", "36", "37",
        "38", "39", "3A", "3B", "3C", "3D", "3E", "3F",
        "40", "41", "42", "43", "44", "45", "46", "47",
        "48", "49", "4A", "4B", "4C", "4D", "4E", "4F",
        "50", "51", "52", "53", "54", "55", "56", "57",
        "58", "59", "5A", "5B", "5C", "5D", "5E", "5F",
        "60", "61", "62", "63", "64", "65", "66", "67",
        "68", "69", "6A", "6B", "6C", "6D", "6E", "6F",
        "70", "71", "72", "73", "74", "75", "76", "77",
        "78", "79", "7A", "7B", "7C", "7D", "7E", "7F",
        "80", "81", "82", "83", "84", "85", "86", "87",
        "88", "89", "8A", "8B", "8C", "8D", "8E", "8F",
        "90", "91", "92", "93", "94", "95", "96", "97",
        "98", "99", "9A", "9B", "9C", "9D", "9E", "9F",
        "A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7",
        "A8", "A9", "AA", "AB", "AC", "AD", "AE", "AF",
        "B0", "B1", "B2", "B3", "B4", "B5", "B6", "B7",
        "B8", "B9", "BA", "BB", "BC", "BD", "BE", "BF",
        "C0", "C1", "C2", "C3", "C4", "C5", "C6", "C7",
        "C8", "C9", "CA", "CB", "CC", "CD", "CE", "CF",
        "D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7",
        "D8", "D9", "DA", "DB", "DC", "DD", "DE", "DF",
        "E0", "E1", "E2", "E3", "E4", "E5", "E6", "E7",
        "E8", "E9", "EA", "EB", "EC", "ED", "EE", "EF",
        "F0", "F1", "F2", "F3", "F4", "F5", "F6", "F7",
        "F8", "F9", "FA", "FB", "FC", "FD", "FE", "FF"
    };
    switch (charset) {
    case LITTON_CHARSET_ASCII:
    case LITTON_CHARSET_UASCII:
        *string_form = 0;
        break;

    case LITTON_CHARSET_EBS1231:
        *string_form = litton_EBS1231_to_ASCII[ch & 0x7F];
        if ((*string_form)[1] == '\0') {
            /* Single character is converted directly to ASCII */
            return (*string_form)[0];
        } else {
            /* Caller needs to use the string form instead */
            return -2;
        }

    case LITTON_CHARSET_HEX:
        *string_form = hex_bytes[ch & 0xFF];
        return -2;
    }
    return ch;
}

int litton_charset_from_name
    (litton_charset_t *charset, const char *name, size_t name_len)
{
    if (litton_name_match("ASCII", name, name_len)) {
        *charset = LITTON_CHARSET_ASCII;
        return 1;
    }
    if (litton_name_match("UASCII", name, name_len)) {
        *charset = LITTON_CHARSET_UASCII;
        return 1;
    }
    if (litton_name_match("EBS1231", name, name_len)) {
        *charset = LITTON_CHARSET_EBS1231;
        return 1;
    }
    if (litton_name_match("HEX", name, name_len)) {
        *charset = LITTON_CHARSET_HEX;
        return 1;
    }
    return 0;
}

const char *litton_charset_to_name(litton_charset_t charset)
{
    switch (charset) {
    case LITTON_CHARSET_ASCII:      return "ASCII";
    case LITTON_CHARSET_UASCII:     return "UASCII";
    case LITTON_CHARSET_EBS1231:    return "EBS1231";
    case LITTON_CHARSET_HEX:        return "HEX";
    }
    return "ASCII"; /* Just in case */
}

uint8_t litton_print_wheel_position(uint8_t code)
{
    if (code >= 0101 && code <= 0177) {
        return (code - 0101) * 3 + 4;
    } else {
        return 0;
    }
}

int litton_is_valid_device_id(uint8_t id)
{
    /* Either bit 6 or 7 must be non-zero */
    if ((id & 0xC0) == 0) {
        return 0;
    }

    /* Any of bits 0 to 5 must be non-zero */
    return (id & 0x3F) != 0;
}
