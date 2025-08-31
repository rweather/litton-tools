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

void litton_init(litton_state_t *state, litton_drum_loc_t size)
{
    /* Clear the machine state */
    memset(state, 0, sizeof(litton_state_t));

    /* Set the size of the memory drum */
    if (size >= (LITTON_DRUM_NUM_TRACKS * LITTON_DRUM_NUM_SECTORS)) {
        size = LITTON_DRUM_NUM_TRACKS * LITTON_DRUM_NUM_SECTORS;
    }
    state->memory_size = size;

    /* Set the last word of memory to "Halt", "Jump to Last", "Halt".
     * This will halt the machine immediately.  If it resumes, it will
     * jump back to the last word and halt again. */
    size = size - 1;
    state->drum[size - 1] = 0x0000E00000ULL;
    state->drum[size - 1] |= ((litton_word_t)size) << 8;
    state->drum[size - 1] |= ((litton_word_t)(size & 0xFF)) << 32;
    state->PC = size;

    /* Reset the machine, which will effect a jump to 0xFFF */
    litton_reset(state);
}

void litton_reset(litton_state_t *state)
{
    /* Force an unconditional jump to the last word of memory into CR and I */
    litton_drum_loc_t end = state->memory_size - 1;
    state->CR = 0xE0 | (end >> 8);
    state->I = ((litton_word_t)(end & 0xFFU)) << 32;
    state->PC = end;
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
