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
#include <string.h>

void litton_init(litton_state_t *state)
{
    /* Clear the machine state */
    memset(state, 0, sizeof(litton_state_t));

    /* Set the default entry point at reset time to the last word in memory */
    state->entry_point = LITTON_DRUM_MAX_SIZE - 1;

    /* Set the default drum size */
    litton_set_drum_size(state, LITTON_DRUM_MAX_SIZE);

    /* Reset the machine, which will effect a jump to the entry point */
    litton_reset(state);
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
    /* Force an unconditional jump to the entry point into CR and I */
    litton_drum_loc_t entry = state->entry_point;
    state->CR = 0xE0 | (entry >> 8);
    state->I = ((litton_word_t)(entry & 0xFFU)) << 32;
    state->PC = entry;
}

litton_word_t litton_get_scratchpad(litton_state_t *state, uint8_t S)
{
    return state->drum[S];
}

void litton_set_scratchpad
    (litton_state_t *state, uint8_t S, litton_word_t value)
{
    state->drum[S] = value;
}
