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

uint32_t litton_get_status_lights(litton_state_t *state)
{
    return state->status_lights;
}

static void litton_modify_register_word
    (litton_word_t *word, unsigned bit, uint8_t mask, uint8_t value)
{
    *word &= ~(((litton_word_t)mask) << bit);
    *word |= (((litton_word_t)value) << bit);
}

static void litton_modify_register
    (litton_state_t *state, uint8_t mask, uint8_t value)
{
    value &= mask;
    switch (state->selected_register) {
    case LITTON_BUTTON_CONTROL_UP:
    case LITTON_BUTTON_CONTROL_DOWN:
        state->CR = (state->CR & ~mask) | value;
        break;

    case LITTON_BUTTON_INST_32:
        litton_modify_register_word(&(state->I), 32, mask, value);
        break;

    case LITTON_BUTTON_INST_24:
        litton_modify_register_word(&(state->I), 24, mask, value);
        break;

    case LITTON_BUTTON_INST_16:
        litton_modify_register_word(&(state->I), 16, mask, value);
        break;

    case LITTON_BUTTON_INST_8:
        litton_modify_register_word(&(state->I), 8, mask, value);
        break;

    case LITTON_BUTTON_INST_0:
        litton_modify_register_word(&(state->I), 0, mask, value);
        break;

    case LITTON_BUTTON_ACCUM_32:
        litton_modify_register_word(&(state->A), 32, mask, value);
        break;

    case LITTON_BUTTON_ACCUM_24:
        litton_modify_register_word(&(state->A), 24, mask, value);
        break;

    case LITTON_BUTTON_ACCUM_16:
        litton_modify_register_word(&(state->A), 16, mask, value);
        break;

    case LITTON_BUTTON_ACCUM_8:
        litton_modify_register_word(&(state->A), 8, mask, value);
        break;

    case LITTON_BUTTON_ACCUM_0:
        litton_modify_register_word(&(state->A), 0, mask, value);
        break;
    }
}

int litton_press_button(litton_state_t *state, uint32_t button)
{
    int ok = 1;

    /* If the power is off, then the only valid button is to turn it on.
     * All other buttons are non-operative. */
    if (button != LITTON_BUTTON_POWER &&
            (state->status_lights & LITTON_STATUS_POWER) == 0) {
        state->selected_register = LITTON_BUTTON_CONTROL_UP;
        return 0;
    }

    /* Pressing any button clears the halt code display mode */
    state->status_lights &= ~LITTON_STATUS_HALT_CODE;

    /* Determine what to do based on the button */
    switch (button) {
    case LITTON_BUTTON_POWER:
        /* Turn the power on or off */
        if ((state->status_lights & LITTON_STATUS_POWER) == 0) {
            /* Power is off, turn it on and go into halt */
            state->status_lights = LITTON_STATUS_POWER | LITTON_STATUS_HALT;
            litton_reset(state);
        } else {
            /* Power is on, so turn it off */
            state->status_lights = 0;
            state->selected_register = LITTON_BUTTON_CONTROL_UP;
            return 1;
        }
        break;

    case LITTON_BUTTON_READY:
        /* If the machine is not ready, then make it so */
        if ((state->status_lights & LITTON_STATUS_READY) == 0) {
            /* Make the machine ready and reset it */
            state->status_lights |= LITTON_STATUS_READY;
            litton_reset(state);
        } else if ((state->status_lights & LITTON_STATUS_RUN) == 0) {
            /* If the machine is halted, then READY will reset it */
            litton_reset(state);
        } else {
            /* READY button does nothing if the machine is running */
            ok = 0;
        }
        break;

    case LITTON_BUTTON_RUN:
        /* Run requires the system to be ready. */
        if ((state->status_lights & LITTON_STATUS_READY) == 0) {
            ok = 0;
            break;
        }

        /* Start the machine running if it is halted.  If the machine is
         * already running, then nothing to do. */
        if ((state->status_lights & LITTON_STATUS_RUN) == 0) {
            state->status_lights |= LITTON_STATUS_RUN;
            state->status_lights &= ~LITTON_STATUS_HALT;
            if (state->CR == LOP_HH) {
                /* If the current instruction is halt, then replace it
                 * with no-op to skip over the halt. */
                state->CR = LOP_NN;
            }

            /* Move the knob back to control up if not control down.
             * Cannot be set to anything except control when running. */
            if (state->selected_register != LITTON_BUTTON_CONTROL_DOWN) {
                state->selected_register = LITTON_BUTTON_CONTROL_UP;
            }
        }
        break;

    case LITTON_BUTTON_HALT:
        /* Halt requires the register select switch to be set to control
         * and the system must be ready. */
        if (state->selected_register != LITTON_BUTTON_CONTROL_UP &&
                state->selected_register != LITTON_BUTTON_CONTROL_DOWN) {
            ok = 0;
            break;
        }
        if ((state->status_lights & LITTON_STATUS_READY) == 0) {
            ok = 0;
            break;
        }

        /* Halt the machine if it is currently running, or single-step a
         * single instruction if it is not running. */
        if ((state->status_lights & LITTON_STATUS_RUN) != 0) {
            state->status_lights &= ~LITTON_STATUS_RUN;
            state->status_lights |= LITTON_STATUS_HALT;
        } else {
            litton_step(state);
        }
        break;

    case LITTON_BUTTON_K_RESET:
    case LITTON_BUTTON_K_SET:
        /* Set or reset the state of K, must be halted and ready */
        if ((state->status_lights & LITTON_STATUS_RUN) == 0 &&
                (state->status_lights & LITTON_STATUS_READY) != 0) {
            state->K = (button == LITTON_BUTTON_K_SET);
        } else {
            ok = 0;
        }
        break;

    case LITTON_BUTTON_RESET:
        /* Reset the currently-selected register if halted and ready */
        if ((state->status_lights & LITTON_STATUS_RUN) == 0 &&
                (state->status_lights & LITTON_STATUS_READY) != 0) {
            litton_modify_register(state, 0xFF, 0x00);
        } else {
            ok = 0;
        }
        break;

    case LITTON_BUTTON_BIT_0:
    case LITTON_BUTTON_BIT_1:
    case LITTON_BUTTON_BIT_2:
    case LITTON_BUTTON_BIT_3:
    case LITTON_BUTTON_BIT_4:
    case LITTON_BUTTON_BIT_5:
    case LITTON_BUTTON_BIT_6:
    case LITTON_BUTTON_BIT_7:
        /* Set a bit in the currently selected register if halted and ready */
        if ((state->status_lights & LITTON_STATUS_RUN) == 0 &&
                (state->status_lights & LITTON_STATUS_READY) != 0) {
            litton_modify_register(state, (button >> 8) & 0xFF, 0xFF);
        } else {
            ok = 0;
        }
        break;

    case LITTON_BUTTON_CONTROL_UP:
    case LITTON_BUTTON_INST_32:
    case LITTON_BUTTON_INST_24:
    case LITTON_BUTTON_INST_16:
    case LITTON_BUTTON_INST_8:
    case LITTON_BUTTON_INST_0:
    case LITTON_BUTTON_CONTROL_DOWN:
    case LITTON_BUTTON_ACCUM_32:
    case LITTON_BUTTON_ACCUM_24:
    case LITTON_BUTTON_ACCUM_16:
    case LITTON_BUTTON_ACCUM_8:
    case LITTON_BUTTON_ACCUM_0:
        /* Adjust the position of the register selector switch.
         * If the machine is halted, this will also update the
         * register display lights.  No change if running. */
        if ((state->status_lights & LITTON_STATUS_RUN) == 0 &&
                (state->status_lights & LITTON_STATUS_READY) != 0) {
            state->selected_register = button;
        }
        break;

    default:
        /* Unknown button */
        ok = 0;
        break;
    }

    /* Update the status lights to reflect the selected register or
     * running instruction. */
    litton_update_status_lights(state);
    return ok;
}

int litton_is_halted(litton_state_t *state)
{
    return (state->status_lights & LITTON_STATUS_RUN) == 0;
}

static void litton_update_register_display(litton_state_t *state, uint8_t value)
{
    state->status_lights &=
        ~(LITTON_STATUS_BIT_0 | LITTON_STATUS_BIT_1 | LITTON_STATUS_BIT_2 |
          LITTON_STATUS_BIT_3 | LITTON_STATUS_BIT_4 | LITTON_STATUS_BIT_5 |
          LITTON_STATUS_BIT_6 | LITTON_STATUS_BIT_7);
    state->status_lights |= ((uint32_t)value) << 8;
}

void litton_update_status_lights(litton_state_t *state)
{
    /* Nothing to do if the power is off */
    if ((state->status_lights & LITTON_STATUS_POWER) == 0) {
        state->status_lights = 0;
        return;
    }

    /* Show the state of K on the lights */
    if (state->K) {
        state->status_lights |= LITTON_STATUS_K;
    } else {
        state->status_lights &= ~LITTON_STATUS_K;
    }

    /* Show the low bit of the current track number on the TRACK light */
    if ((state->last_address & 0x0080) != 0) {
        state->status_lights |= LITTON_STATUS_TRACK;
    } else {
        state->status_lights &= ~LITTON_STATUS_TRACK;
    }

    /* The register display shows CR when running, or the currently
     * selected register on the control knob when halted */
    if ((state->status_lights & LITTON_STATUS_RUN) != 0) {
        /* Show the contents of CR */
        litton_update_register_display(state, state->CR);

        /* Accumulator and instruction lights are off when running */
        state->status_lights &= ~LITTON_STATUS_ACCUM;
        state->status_lights &= ~LITTON_STATUS_INST;
    } else if ((state->status_lights & LITTON_STATUS_HALT_CODE) != 0) {
        /* Displaying the halt code just after the program halted.
         * As soon as a button is pressed, the halt code will go away. */
        litton_update_register_display(state, state->halt_code);
        state->status_lights &= ~LITTON_STATUS_ACCUM;
        state->status_lights &= ~LITTON_STATUS_INST;
    } else {
        /* Determine which register to display */
        switch (state->selected_register) {
        case LITTON_BUTTON_CONTROL_UP:
        case LITTON_BUTTON_CONTROL_DOWN:
            litton_update_register_display(state, state->CR);
            state->status_lights &= ~LITTON_STATUS_ACCUM;
            state->status_lights &= ~LITTON_STATUS_INST;
            break;

        case LITTON_BUTTON_INST_32:
            litton_update_register_display(state, state->I >> 32);
            state->status_lights &= ~LITTON_STATUS_ACCUM;
            state->status_lights |= LITTON_STATUS_INST;
            break;

        case LITTON_BUTTON_INST_24:
            litton_update_register_display(state, state->I >> 24);
            state->status_lights &= ~LITTON_STATUS_ACCUM;
            state->status_lights |= LITTON_STATUS_INST;
            break;

        case LITTON_BUTTON_INST_16:
            litton_update_register_display(state, state->I >> 16);
            state->status_lights &= ~LITTON_STATUS_ACCUM;
            state->status_lights |= LITTON_STATUS_INST;
            break;

        case LITTON_BUTTON_INST_8:
            litton_update_register_display(state, state->I >> 8);
            state->status_lights &= ~LITTON_STATUS_ACCUM;
            state->status_lights |= LITTON_STATUS_INST;
            break;

        case LITTON_BUTTON_INST_0:
            litton_update_register_display(state, state->I);
            state->status_lights &= ~LITTON_STATUS_ACCUM;
            state->status_lights |= LITTON_STATUS_INST;
            break;

        case LITTON_BUTTON_ACCUM_32:
            litton_update_register_display(state, state->A >> 32);
            state->status_lights |= LITTON_STATUS_ACCUM;
            state->status_lights &= ~LITTON_STATUS_INST;
            break;

        case LITTON_BUTTON_ACCUM_24:
            litton_update_register_display(state, state->A >> 24);
            state->status_lights |= LITTON_STATUS_ACCUM;
            state->status_lights &= ~LITTON_STATUS_INST;
            break;

        case LITTON_BUTTON_ACCUM_16:
            litton_update_register_display(state, state->A >> 16);
            state->status_lights |= LITTON_STATUS_ACCUM;
            state->status_lights &= ~LITTON_STATUS_INST;
            break;

        case LITTON_BUTTON_ACCUM_8:
            litton_update_register_display(state, state->A >> 8);
            state->status_lights |= LITTON_STATUS_ACCUM;
            state->status_lights &= ~LITTON_STATUS_INST;
            break;

        case LITTON_BUTTON_ACCUM_0:
            litton_update_register_display(state, state->A);
            state->status_lights |= LITTON_STATUS_ACCUM;
            state->status_lights &= ~LITTON_STATUS_INST;
            break;
        }
    }
}
