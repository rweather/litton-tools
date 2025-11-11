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

#include "litton/litton.h"
#include <stdlib.h>
#include <string.h>
#if defined(__AVR__)
#include <avr/pgmspace.h>
#endif

void litton_clear_memory(litton_state_t *state)
{
    litton_drum_loc_t addr;

    /* Clear the drum */
    for (addr = 0; addr < LITTON_DRUM_MAX_SIZE; ++addr) {
        litton_set_memory(state, addr, 0);
    }

    /* Set the default entry point at reset time to the last word in memory */
    state->entry_point = LITTON_DRUM_MAX_SIZE - 1;

    /* Set the default drum size */
    litton_set_drum_size(state, LITTON_DRUM_MAX_SIZE);

    /* Set the default device information for the printer and keyboard */
    state->printer_id = LITTON_DEVICE_PRINTER;
    state->printer_charset = LITTON_CHARSET_EBS1231;
    state->keyboard_id = LITTON_DEVICE_KEYBOARD;
    state->keyboard_charset = LITTON_CHARSET_EBS1231;

    /* Select register is control up to begin with */
    state->selected_register = LITTON_BUTTON_CONTROL_UP;

    /* Start with the power on and the machine halted */
    state->status_lights = LITTON_STATUS_POWER | LITTON_STATUS_HALT;

    /* Reset the machine, which will effect a jump to the entry point */
    litton_reset(state);
}

void litton_init(litton_state_t *state)
{
    memset(state, 0, sizeof(litton_state_t));
    litton_clear_memory(state);
}

void litton_free(litton_state_t *state)
{
    /* Close all of the attached devices */
    litton_device_t *device = state->devices;
    litton_device_t *next_device;
    while (device != 0) {
        next_device = device->next;
        if (device->close != 0) {
            (*(device->close))(state, device);
        }
        if (device->file) {
            fclose(device->file);
        }
        free(device);
        device = next_device;
    }

    /* Clear the machine state */
    memset(state, 0, sizeof(litton_state_t));
}

void litton_set_drum_size(litton_state_t *state, litton_drum_loc_t size)
{
    /* Range-check the size, just in case */
    if (size == 0 || size > LITTON_DRUM_MAX_SIZE) {
        size = LITTON_DRUM_MAX_SIZE;
    }
    state->drum_size = size;

    /* Adjust the entry point if it is now beyond the end of the drum */
    if (state->entry_point >= size) {
        state->entry_point = size - 1;
    }
}

void litton_set_entry_point(litton_state_t *state, litton_drum_loc_t entry)
{
    if (entry >= state->drum_size) {
        entry = state->drum_size - 1;
    }
    state->entry_point = entry;
}

void litton_reset(litton_state_t *state)
{
    /* Force a conditional jump to the entry point into CR and I */
    litton_drum_loc_t entry = state->entry_point;
    state->CR = 0xF0 | (entry >> 8);
    state->I = ((litton_word_t)(entry & 0xFFU)) << 32;
    state->I |= 0xFFFFFFFFU; /* Set the rest of I to FF's */
    state->last_address = state->entry_point;

    /* Fake the jump to the entry point as starting at 0xFFF */
    state->PC = LITTON_DRUM_MAX_SIZE - 1;

    /* A is set to all-1's upon reset */
    state->A = LITTON_WORD_MASK;

    /* K is set to 1 upon reset */
    state->K = 1;
}

#if LITTON_SMALL_MEMORY

static uint8_t *litton_get_track
    (litton_state_t *state, litton_drum_loc_t addr)
{
    switch (addr >> 7) {
    case 0:  return state->track0;
    case 6:  return state->track6;
    case 7:  return state->track7;
    case 9:  return state->track9;
    case 16: return state->track16;
    case 17: return state->track17;
    default: return 0;
    }
}

#if defined(__AVR__)
extern unsigned char const litton_opus[];
#else
extern const litton_word_t * const litton_opus;
#endif

#endif

litton_word_t litton_get_memory(litton_state_t *state, litton_drum_loc_t addr)
{
#if LITTON_SMALL_MEMORY
    uint8_t *ptr;
    litton_word_t value;
    if (addr < LITTON_DRUM_RESERVED_SECTORS) {
        /* Read from the scratchpad loop instead of main memory */
        return state->scratchpad[addr];
    }
    ptr = litton_get_track(state, addr);
    if (ptr) {
        /* Read from a writable track */
        ptr += (addr & (LITTON_DRUM_NUM_SECTORS - 1)) * 5;
#if defined(__AVR__)
        value = 0;
        memcpy(&value, ptr, 5);
#else
        value = ((litton_word_t)(ptr[0])) |
               (((litton_word_t)(ptr[1])) << 8) |
               (((litton_word_t)(ptr[2])) << 16) |
               (((litton_word_t)(ptr[3])) << 24) |
               (((litton_word_t)(ptr[4])) << 32);
#endif
        return value;
    } else {
        /* Read directly from the OPUS image in flash memory */
#if defined(__AVR__)
        litton_word_t word = 0;
        memcpy_P(&word, litton_opus + (addr & (LITTON_DRUM_MAX_SIZE - 1)) * 5, 5);
        return word;
#else
        return litton_opus[addr & (LITTON_DRUM_MAX_SIZE - 1)];
#endif
    }
#else
    return state->drum[addr & (LITTON_DRUM_MAX_SIZE - 1)];
#endif
}

void litton_set_memory
    (litton_state_t *state, litton_drum_loc_t addr, litton_word_t value)
{
#if LITTON_SMALL_MEMORY
    uint8_t *ptr;
    if (addr < LITTON_DRUM_RESERVED_SECTORS) {
        /* Write to the scratchpad loop instead of main memory */
        state->scratchpad[addr] = value;
    }
    ptr = litton_get_track(state, addr);
    if (ptr) {
        ptr += (addr & (LITTON_DRUM_NUM_SECTORS - 1)) * 5;
#if defined(__AVR__)
        memcpy(ptr, &value, 5);
#else
        ptr[0] = (uint8_t)value;
        ptr[1] = (uint8_t)(value >> 8);
        ptr[2] = (uint8_t)(value >> 16);
        ptr[3] = (uint8_t)(value >> 24);
        ptr[4] = (uint8_t)(value >> 32);
#endif
    }
#else
    state->drum[addr & (LITTON_DRUM_MAX_SIZE - 1)] = value;
#endif
}

litton_word_t litton_get_scratchpad(litton_state_t *state, uint8_t S)
{
#if LITTON_SMALL_MEMORY
    return state->scratchpad[S & (LITTON_DRUM_RESERVED_SECTORS - 1)];
#else
    return litton_get_memory(state, S & (LITTON_DRUM_RESERVED_SECTORS - 1));
#endif
}

void litton_set_scratchpad
    (litton_state_t *state, uint8_t S, litton_word_t value)
{
#if LITTON_SMALL_MEMORY
    state->scratchpad[S & (LITTON_DRUM_RESERVED_SECTORS - 1)] = value;
#else
    litton_set_memory(state, S & (LITTON_DRUM_RESERVED_SECTORS - 1), value);
#endif
}

litton_word_t *litton_get_scratchpad_address(litton_state_t *state, uint8_t S)
{
#if LITTON_SMALL_MEMORY
    return &(state->scratchpad[S & (LITTON_DRUM_RESERVED_SECTORS - 1)]);
#else
    return &(state->drum[S & (LITTON_DRUM_RESERVED_SECTORS - 1)]);
#endif
}

int litton_name_match(const char *name1, const char *name2, size_t name2_len)
{
    int ch1, ch2;
    while (*name1 != '\0' && name2_len > 0) {
        ch1 = *name1++;
        if (ch1 >= 'a' && ch1 <= 'z') {
            ch1 = ch1 - 'a' + 'A';
        }
        ch2 = *name2++;
        if (ch2 >= 'a' && ch2 <= 'z') {
            ch2 = ch2 - 'a' + 'A';
        }
        if (ch1 != ch2) {
            return 0;
        }
        --name2_len;
    }
    if (*name1 != '\0' || name2_len != 0) {
        return 0;
    } else {
        return 1;
    }
}
