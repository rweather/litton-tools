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

void litton_remove_device(litton_state_t *state, litton_device_t *device)
{
    litton_device_t *current;
    litton_device_t **prev;
    litton_deselect_device(state, device);
    current = state->devices;
    prev = &(state->devices);
    while (current != 0) {
        if (current == device) {
            *prev = current->next;
            device->next = 0;
            return;
        }
        prev = &(current->next);
        current = current->next;
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

static void litton_console_output
    (litton_state_t *state, litton_device_t *device,
     uint8_t value, litton_parity_t parity)
{
    int ch;
    (void)state;
    value = litton_remove_parity(value, parity);
    ch = litton_char_from_charset(value, device->charset);
    if (ch != -1) {
        putc(ch, stdout);
    }
}

void litton_add_console
    (litton_state_t *state, uint8_t id, litton_charset_t charset)
{
    litton_device_t *device = calloc(1, sizeof(litton_device_t));
    device->id = id;
    device->supports_input = 0; /* TODO: Console input */
    device->supports_output = 1;
    device->charset = charset;
    device->output = litton_console_output;
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

int litton_char_to_charset(int ch, litton_charset_t charset)
{
    // TODO: Non-ASCII character sets.
    (void)charset;
    return ch;
}

int litton_char_from_charset(int ch, litton_charset_t charset)
{
    // TODO: Non-ASCII character sets.
    (void)charset;
    return ch;
}
