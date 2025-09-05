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

static uint16_t litton_available_scratchpad(litton_state_t *state)
{
    /*
     * Some of the shift instructions refer to "whichever scratchpad register
     * is available for use".  It isn't clear which register that is.
     *
     * Until we figure out what the rule is, always use scratchpad register 0.
     */
    (void)state;
    return 0;
}

static void litton_single_left_shift
    (litton_state_t *state, litton_word_t *word, litton_word_t K, uint16_t N)
{
    litton_word_t A = *word;
    while (N > 0) {
        A = (A << 1) | K;
        K = (A >> LITTON_WORD_BITS);
        A &= LITTON_WORD_MASK;
        --N;
    }
    *word = A;
    state->K = K;
}

static void litton_double_left_shift
    (litton_state_t *state, litton_word_t *word1, litton_word_t *word2,
     litton_word_t K, uint16_t N)
{
    litton_word_t A = *word1;
    litton_word_t B = *word2;
    while (N > 0) {
        B = (B << 1) | K;
        K = (B >> LITTON_WORD_BITS);
        B &= LITTON_WORD_MASK;
        A = (A << 1) | K;
        K = (A >> LITTON_WORD_BITS);
        A &= LITTON_WORD_MASK;
        --N;
    }
    *word1 = A;
    *word2 = B;
    state->K = K;
}

static void litton_single_right_shift
    (litton_state_t *state, litton_word_t *word, litton_word_t K, uint16_t N)
{
    litton_word_t A = *word;
    litton_word_t nextK;
    while (N > 0) {
        nextK = A & 1;
        A = (A >> 1) | (K << (LITTON_WORD_BITS - 1));
        K = nextK;
        --N;
    }
    *word = A;
    state->K = K;
}

static void litton_double_right_shift
    (litton_state_t *state, litton_word_t *word1, litton_word_t *word2,
     litton_word_t K, uint16_t N)
{
    litton_word_t A = *word1;
    litton_word_t B = *word2;
    litton_word_t nextK;
    while (N > 0) {
        nextK = A & 1;
        A = (A >> 1) | (K << (LITTON_WORD_BITS - 1));
        K = nextK;
        nextK = B & 1;
        B = (B >> 1) | (K << (LITTON_WORD_BITS - 1));
        K = nextK;
        --N;
    }
    *word1 = A;
    *word2 = B;
    state->K = K;
}

static litton_step_result_t litton_binary_shift
    (litton_state_t *state, uint16_t insn)
{
    uint16_t S = litton_available_scratchpad(state);
    litton_word_t K = 0;
    if ((insn & 0x0080) != 0) {
        /* Shift with carry */
        K = state->K;
    }
    switch (insn & 0xFF80) {
    case LOP_BLS:
    case LOP_BLSK:
        /* Binary left single shift of A */
        litton_single_left_shift(state, &(state->A), K, (insn & 0x7F) + 1);
        break;

    case LOP_BLSS:
    case LOP_BLSSK:
        /* Binary left single shift of scratchpad register */
        if ((insn & 0x7F) != 0) {
            return LITTON_STEP_ILLEGAL;
        }
        litton_single_left_shift(state, &(state->drum[S]), K, 1);
        break;

    case LOP_BLD:
    case LOP_BLDK:
        /* Binary left double shift of S0/S1 */
        litton_double_left_shift
            (state, &(state->drum[0]), &(state->drum[1]), K, (insn & 0x7F) + 1);
        break;

    case LOP_BLDS:
    case LOP_BLDSK:
        /* Binary left double shift of scratchpad register */
        if ((insn & 0x7F) != 0) {
            return LITTON_STEP_ILLEGAL;
        }
        litton_double_left_shift
            (state, &(state->drum[S]), &(state->drum[(S + 1) & 0x07]), K, 1);
        break;

    case LOP_BRS:
    case LOP_BRSK:
        /* Binary right single shift of A */
        litton_single_right_shift(state, &(state->A), K, (insn & 0x7F) + 1);
        break;

    case LOP_BRSS:
    case LOP_BRSSK:
        /* Binary right single shift of scratchpad register */
        if ((insn & 0x7F) != 0) {
            return LITTON_STEP_ILLEGAL;
        }
        litton_single_right_shift(state, &(state->drum[S]), K, 1);
        break;

    case LOP_BRD:
    case LOP_BRDK:
        /* Binary right double shift of S0/S1 */
        litton_double_right_shift
            (state, &(state->drum[0]), &(state->drum[1]), K, (insn & 0x7F) + 1);
        break;

    case LOP_BRDS:
    case LOP_BRDSK:
        /* Binary right double shift of scratchpad register */
        if ((insn & 0x7F) != 0) {
            return LITTON_STEP_ILLEGAL;
        }
        litton_double_right_shift
            (state, &(state->drum[S]), &(state->drum[(S + 1) & 0x07]), K, 1);
        break;

    default:
        /* Not a valid binary shift instruction */
        return LITTON_STEP_ILLEGAL;
    }
    return LITTON_STEP_OK;
}

static void litton_single_decimal_left_shift
    (litton_state_t *state, litton_word_t *word, litton_word_t K, uint16_t N)
{
    litton_word_t A = *word;
    while (N > 0) {
        A = A * 10 + K;
        A &= LITTON_WORD_MASK;
        K = 0;
        --N;
    }
    *word = A;
    state->K = K;
}

static void litton_single_decimal_right_shift
    (litton_state_t *state, litton_word_t *word, uint16_t N)
{
    litton_word_t A = *word;
    while (N > 0) {
        A = A / 10;
        --N;
    }
    *word = A;
    state->K = 0;
}

static void litton_double_times_2
    (litton_word_t *word1, litton_word_t *word2)
{
    *word1 <<= 1;
    *word2 <<= 1;
    *word1 += (*word2) >> LITTON_WORD_BITS;
    *word1 &= LITTON_WORD_MASK;
    *word2 &= LITTON_WORD_MASK;
}

static void litton_double_mul_10
    (litton_word_t *word1, litton_word_t *word2)
{
    litton_word_t tword1, tword2;

    /* Multiply an 80-bit number by 10 using bit shifts and adds */
    litton_double_times_2(word1, word2);
    tword1 = *word1;
    tword2 = *word2;
    litton_double_times_2(word1, word2);
    litton_double_times_2(word1, word2);
    *word1 += tword1;
    *word2 += tword2;

    /* Account for the carry from the low word and mask off the words */
    *word1 += (*word2) >> LITTON_WORD_BITS;
    *word1 &= LITTON_WORD_MASK;
    *word2 &= LITTON_WORD_MASK;
}

static void litton_double_decimal_left_shift
    (litton_state_t *state, litton_word_t *word1, litton_word_t *word2,
     litton_word_t K, uint16_t N)
{
    litton_word_t A = *word1;
    litton_word_t B = *word2;
    while (N > 0) {
        litton_double_mul_10(&A, &B);
        B += K;
        K = 0;
        --N;
    }
    *word1 = A;
    *word2 = B;
    state->K = K;
}

static void litton_double_div_10
    (litton_word_t *word1, litton_word_t *word2)
{
    // TODO: Divide an 80-bit number by 10.
    (void)word1;
    (void)word2;
}

static void litton_double_decimal_right_shift
    (litton_state_t *state, litton_word_t *word1, litton_word_t *word2,
     uint16_t N)
{
    litton_word_t A = *word1;
    litton_word_t B = *word2;
    while (N > 0) {
        litton_double_div_10(&A, &B);
        --N;
    }
    *word1 = A;
    *word2 = B;
    state->K = 0;
}

static litton_step_result_t litton_decimal_shift
    (litton_state_t *state, uint16_t insn)
{
    uint16_t S = litton_available_scratchpad(state);
    switch (insn & 0xFF80) {
    case LOP_DLS:
        /* Decimal left single shift of A */
        litton_single_decimal_left_shift
            (state, &(state->A), 0, (insn & 0x7F) + 1);
        break;

    case LOP_DLSC:
        /* Decimal left single shift of A, plus constant */
        litton_single_decimal_left_shift
            (state, &(state->A), 1, (insn & 0x7F) + 1);
        break;

    case LOP_DLSS:
        /* Decimal left single shift of scratchpad register */
        if ((insn & 0x7F) != 0) {
            return LITTON_STEP_ILLEGAL;
        }
        litton_single_decimal_left_shift(state, &(state->drum[S]), 0, 1);
        break;

    case LOP_DLSSC:
        /* Decimal left single shift of scratchpad register, plus constant */
        if ((insn & 0x7F) != 0) {
            return LITTON_STEP_ILLEGAL;
        }
        litton_single_decimal_left_shift(state, &(state->drum[S]), 1, 1);
        break;

    case LOP_DLD:
        /* Decimal left double shift of S0/S1 */
        litton_double_decimal_left_shift
            (state, &(state->drum[0]), &(state->drum[1]), 0, (insn & 0x7F) + 1);
        break;

    case LOP_DLDC:
        /* Decimal left double shift of S0/S1, plus constant */
        litton_double_decimal_left_shift
            (state, &(state->drum[0]), &(state->drum[1]), 1, (insn & 0x7F) + 1);
        break;

    case LOP_DLDS:
        /* Decimal left double shift of scratchpad register */
        if ((insn & 0x7F) != 0) {
            return LITTON_STEP_ILLEGAL;
        }
        litton_double_decimal_left_shift
            (state, &(state->drum[S]), &(state->drum[(S + 1) & 0x07]), 0, 1);
        break;

    case LOP_DLDSC:
        /* Decimal left double shift of scratchpad register, plus constant*/
        if ((insn & 0x7F) != 0) {
            return LITTON_STEP_ILLEGAL;
        }
        litton_double_decimal_left_shift
            (state, &(state->drum[S]), &(state->drum[(S + 1) & 0x07]), 1, 1);
        break;

    case LOP_DRS:
        /* Decimal right single shift of A */
        litton_single_decimal_right_shift
            (state, &(state->A), (insn & 0x7F) + 1);
        break;

    case LOP_DRD:
        /* Decimal right double shift of S0/S1 */
        litton_double_decimal_right_shift
            (state, &(state->drum[0]), &(state->drum[1]), (insn & 0x7F) + 1);
        break;

    default:
        /* Not a valid decimal shift instruction */
        return LITTON_STEP_ILLEGAL;
    }
    return LITTON_STEP_OK;
}

static void litton_parity_check
    (litton_state_t *state, uint8_t value, litton_parity_t parity)
{
    if (litton_add_parity(value, parity) != value) {
        /* Parity check failure */
        state->P = 1;
    }
}

static litton_step_result_t litton_perform_io
    (litton_state_t *state, uint16_t insn)
{
    switch (insn) {
    case LOP_SI:
        /* Shift input.  The reference manual implies that parity errors
         * can occur from this command, but there is no way to specify the
         * parity that is expected.  Assume no parity instead. */
        if (litton_input_from_device(state, &(state->B), LITTON_PARITY_NONE)) {
            /* Shift the received byte into A */
            state->A = (state->A << 8) | state->B;
            state->B = state->A >> LITTON_WORD_BITS;
            state->A &= LITTON_WORD_MASK;
            state->K = 1;
        } else {
            /* Input device is currently busy */
            state->K = 0;
        }
        break;

    case LOP_RS:
        /* Read status */
        if (litton_input_device_status(state, &(state->B))) {
            /* Shift the received status byte into A */
            state->A = (state->A << 8) | state->B;
            state->B = state->A >> LITTON_WORD_BITS;
            state->A &= LITTON_WORD_MASK;
            state->K = 1;
        } else {
            /* Input device is currently busy */
            state->K = 0;
        }
        break;

    case LOP_CIO:
        /* Clear, input, check odd parity */
        if (litton_input_from_device(state, &(state->B), LITTON_PARITY_ODD)) {
            /* Shift the received byte into A */
            litton_parity_check(state, state->B, LITTON_PARITY_ODD);
            state->A = litton_remove_parity(state->B, LITTON_PARITY_ODD);
            state->B = 0;
            state->K = 1;
        } else {
            /* Input device is currently busy */
            state->K = 0;
        }
        break;

    case LOP_CIE:
        /* Clear, input, check even parity */
        if (litton_input_from_device(state, &(state->B), LITTON_PARITY_EVEN)) {
            /* Shift the received byte into A */
            litton_parity_check(state, state->B, LITTON_PARITY_EVEN);
            state->A = litton_remove_parity(state->B, LITTON_PARITY_EVEN);
            state->B = 0;
            state->K = 1;
        } else {
            /* Input device is currently busy */
            state->K = 0;
        }
        break;

    case LOP_CIOP:
        /* Clear, input, check odd parity into A */
        if (litton_input_from_device(state, &(state->B), LITTON_PARITY_ODD)) {
            /* Shift the received byte into A and record the parity in A */
            litton_parity_check(state, state->B, LITTON_PARITY_ODD);
            state->A = litton_remove_parity(state->B, LITTON_PARITY_ODD);
            if (state->P) {
                state->A |= LITTON_WORD_MSB;
            }
            state->B = 0;
            state->K = 1;
        } else {
            /* Input device is currently busy */
            state->K = 0;
        }
        break;

    case LOP_CIEP:
        /* Clear, input, check even parity into A */
        if (litton_input_from_device(state, &(state->B), LITTON_PARITY_EVEN)) {
            /* Shift the received byte into A and record the parity in A */
            litton_parity_check(state, state->B, LITTON_PARITY_EVEN);
            state->A = litton_remove_parity(state->B, LITTON_PARITY_EVEN);
            if (state->P) {
                state->A |= LITTON_WORD_MSB;
            }
            state->B = 0;
            state->K = 1;
        } else {
            /* Input device is currently busy */
            state->K = 0;
        }
        break;

    case LOP_OAO:
        /* Output accumulator with odd parity */
        if (!litton_is_output_busy(state)) {
            state->B = litton_add_parity(state->A >> 32, LITTON_PARITY_ODD);
            state->A = state->A & 0xFFFFFFFFUL;
            state->A |= ((litton_word_t)(state->B)) << 32;
            state->K = 1;
        } else {
            /* Output device is currently busy */
            state->K = 0;
        }
        break;

    case LOP_OAE:
        /* Output accumulator with even parity */
        if (!litton_is_output_busy(state)) {
            state->B = litton_add_parity(state->A >> 32, LITTON_PARITY_EVEN);
            litton_output_to_device(state, state->B, LITTON_PARITY_EVEN);
            state->A = state->A & 0xFFFFFFFFUL;
            state->A |= ((litton_word_t)(state->B)) << 32;
            state->K = 1;
        } else {
            /* Output device is currently busy */
            state->K = 0;
        }
        break;

    case LOP_OA:
        /* Output accumulator with no parity */
        if (!litton_is_output_busy(state)) {
            state->B = (uint8_t)(state->A >> 32);
            litton_output_to_device(state, state->B, LITTON_PARITY_NONE);
            state->A = state->A & 0xFFFFFFFFUL;
            state->A |= ((litton_word_t)(state->B)) << 32;
            state->K = 1;
        } else {
            /* Output device is currently busy */
            state->K = 0;
        }
        break;

    default:
        /* May be an I/O instruction with an immediate operand */
        switch (insn & 0xFF00) {
        case LOP_OI:
            /* Output immediate */
            if (!litton_is_output_busy(state)) {
                state->B = insn & 0x00FF;
                litton_output_to_device(state, state->B, LITTON_PARITY_NONE);
                state->K = 1;
            } else {
                /* Output device is currently busy */
                state->K = 0;
            }
            break;

        case LOP_IST:
            /* Immediate select on test */
            if (!litton_is_output_busy(state)) {
                state->B = insn & 0x00FF;
                litton_select_device(state, state->B);
                state->K = 1;
            } else {
                /* Output device is currently busy */
                state->K = 0;
            }
            break;

        case LOP_IS:
            /* Immediate select with no test */
            state->B = insn & 0x00FF;
            litton_select_device(state, state->B);
            state->K = 1;
            break;

        default:
            /* Not a valid I/O instruction */
            return LITTON_STEP_ILLEGAL;
        }
    }
    return LITTON_STEP_OK;
}

litton_step_result_t litton_step(litton_state_t *state)
{
    litton_step_result_t result = LITTON_STEP_OK;
    litton_drum_loc_t addr;
    uint16_t insn;
    litton_word_t temp;

    /* Detect a program that is spinning out of control because we
     * haven't seen a jump instruction in a while. */
    if (state->spin_counter > LITTON_DRUM_MAX_SIZE) {
        return LITTON_STEP_SPINNING;
    }
    ++(state->spin_counter);

    if (state->CR < 0x40) {
        /* Single-byte instruction */
        if (state->disassemble) {
            litton_disassemble_instruction(stderr, state->PC, state->CR);
        }
        switch (state->CR) {
        case LOP_HH | 0x00: case LOP_HH | 0x01:
        case LOP_HH | 0x02: case LOP_HH | 0x03:
        case LOP_HH | 0x04: case LOP_HH | 0x05:
        case LOP_HH | 0x06: case LOP_HH | 0x07:
            /* Halt the machine with REGISTER DISPLAY showing the low 3 bits */
            state->register_display = state->CR & 0x07;
            result = LITTON_STEP_HALT;
            break;

        case LOP_AK:
            /* Add K to the accumulator */
            state->A += state->K;
            if (state->A >= LITTON_WORD_MASK) {
                state->A = 0;
                state->K = 1;
            } else {
                state->K = 0;
            }
            break;

        case LOP_CL:
            /* Clear the accumulator */
            state->A = 0;
            break;

        case LOP_NN:
            /* No operation */
            break;

        case LOP_CM:
            /* Complement the accumulator and set K if A is non-zero */
            state->A = (-state->A) & LITTON_WORD_MASK;
            state->K = (state->A != 0);
            break;

        case LOP_JA:
            /* Jump to the contents of the accumulator */
            state->I = state->A;
            break;

        case LOP_BI:
            /* Interchange the Block Interchange Loop with the scratchpad */
            for (addr = 0; addr < LITTON_DRUM_RESERVED_SECTORS; ++addr) {
                temp = state->drum[addr]; /* Scratchpad */
                state->drum[addr] = state->block_interchange_loop[addr];
                state->block_interchange_loop[addr] = temp;
            }

            /* K is set to 0 if an external interchange device is being used
             * and the device is busy.  If the device is ready, set K to 1.
             * We just assume that the block interchange device is ready. */
            state->K = 1;
            break;

        case LOP_SK:
            /* Set K to 1 */
            state->K = 1;
            break;

        case LOP_TZ:
            /* Test A for zero and set K to 1 if it is */
            state->K = (state->A == 0);
            break;

        case LOP_TH:
            /* Test the high bit of A / test for negative */
            state->K = ((state->A & LITTON_WORD_MSB) != 0);
            break;

        case LOP_RK:
            /* Reset K to 0 */
            state->K = 0;
            break;

        case LOP_TP:
            /* Test parity failure and reset the parity failure flag */
            state->K = state->P;
            state->P = 0;
            break;

        case LOP_LA | 0x00: case LOP_LA | 0x01:
        case LOP_LA | 0x02: case LOP_LA | 0x03:
        case LOP_LA | 0x04: case LOP_LA | 0x05:
        case LOP_LA | 0x06: case LOP_LA | 0x07:
            /* Logical AND of scratchpad register S with A */
            state->A &= litton_get_scratchpad(state, state->CR & 0x07);
            state->K = (state->A == 0);
            break;

        case LOP_XC | 0x00: case LOP_XC | 0x01:
        case LOP_XC | 0x02: case LOP_XC | 0x03:
        case LOP_XC | 0x04: case LOP_XC | 0x05:
        case LOP_XC | 0x06: case LOP_XC | 0x07:
            /* Exchange A with scratchpad register S */
            temp = litton_get_scratchpad(state, state->CR & 0x07);
            litton_set_scratchpad(state, state->CR & 0x07, state->A);
            state->A = temp;
            break;

        case LOP_XT | 0x00: case LOP_XT | 0x01:
        case LOP_XT | 0x02: case LOP_XT | 0x03:
        case LOP_XT | 0x04: case LOP_XT | 0x05:
        case LOP_XT | 0x06: case LOP_XT | 0x07:
            /* Extract bits from A and scratchpad register S.
             * The following two statements are executed in parallel:
             *      A = (S & A)
             *      S = (S & ~A)
             */
            temp = litton_get_scratchpad(state, state->CR & 0x07);
            litton_set_scratchpad(state, state->CR & 0x07, temp & ~(state->A));
            state->A &= temp;
            break;

        case LOP_TE | 0x00: case LOP_TE | 0x01:
        case LOP_TE | 0x02: case LOP_TE | 0x03:
        case LOP_TE | 0x04: case LOP_TE | 0x05:
        case LOP_TE | 0x06: case LOP_TE | 0x07:
            /* Test if A is equal to scratchpad register S */
            temp = litton_get_scratchpad(state, state->CR & 0x07);
            state->K = (state->A == temp);
            break;

        case LOP_TG | 0x00: case LOP_TG | 0x01:
        case LOP_TG | 0x02: case LOP_TG | 0x03:
        case LOP_TG | 0x04: case LOP_TG | 0x05:
        case LOP_TG | 0x06: case LOP_TG | 0x07:
            /* Test if A is greater than or equal to scratchpad register S */
            temp = litton_get_scratchpad(state, state->CR & 0x07);
            state->K = (state->A >= temp);
            break;

        default:
            /* Illegal instruction, which we treat like a no-op */
            result = LITTON_STEP_ILLEGAL;
            break;
        }

        /* Rotate CR/I by 8 bits */
        state->I = (state->I << 8) | state->CR;
        state->CR = (uint8_t)(state->I >> LITTON_WORD_BITS);
        state->I &= LITTON_WORD_MASK;
    } else {
        /* Double-byte instruction.  Decide what to do based on the
         * high 4 bits of the command register. */
        insn = (uint16_t)((state->CR << 8) | (state->I >> 32));
        if (state->disassemble) {
            litton_disassemble_instruction(stderr, state->PC, insn);
        }
        addr = insn & 0x0FFF;
        switch (state->CR & 0xF0) {
        case 0x40:
            /* Binary shift instructions */
            result = litton_binary_shift(state, insn);
            break;

        case 0x50:
        case 0x70:
            /* I/O instructions */
            result = litton_perform_io(state, insn);
            break;

        case 0x60:
            /* Decimal shift instructions */
            result = litton_decimal_shift(state, insn);
            break;

        case 0x80:
            /* Load from memory into A */
            state->A = state->drum[addr];
            break;

        case 0x90:
            /* Add memory to A, with carry out in K */
            state->A += state->drum[addr];
            state->K = (state->A >= LITTON_WORD_MASK);
            state->A &= LITTON_WORD_MASK;
            break;

        case 0xB0:
            /* Store A to memory */
            state->drum[addr] = state->A;
            break;

        case 0xC0:
            /* Jump mark command.  This is a type of "jump to subroutine" that
             * saves the return point in A.  When the program later performs a
             * "JA" to A, we will come back to just after the "JM" point. */

            /* Convert the instruction into an unconditional jump for
             * when we rotate it back in again later. */
            state->CR = 0xE0 | (state->CR & 0x0F);

            /* Save the current instruction in A */
            state->A = state->I;
            state->A &= LITTON_WORD_MASK;

            /* Copy the destination instruction into I */
            state->I = state->drum[addr];
            state->PC = addr;
            state->spin_counter = 0;
            break;

        case 0xD0:
            /* Conditional add of memory to A, with carry out in K */
            if (state->K) {
                state->A += state->drum[addr];
                state->K = (state->A >= LITTON_WORD_MASK);
                state->A &= LITTON_WORD_MASK;
            }
            break;

        case 0xE0:
            /* Unconditional jump */
            state->I = state->drum[addr];
            state->PC = addr;
            state->spin_counter = 0;
            break;

        case 0xF0:
            /* Conditional jump */
            if (state->K) {
                /* Jump to the destination address */
                state->I = state->drum[addr];
                state->PC = addr;
                state->spin_counter = 0;

                /* Convert the instruction into an unconditional jump
                 * when we rotate it back in again later. */
                state->CR = 0xE0 | (state->CR & 0x0F);
            }
            break;

        default:
            /* Illegal instruction, which we treat like a no-op */
            result = LITTON_STEP_ILLEGAL;
            break;
        }

        /* Rotate CR/I by 16 bits */
        state->I = (state->I << 8) | (state->CR);
        state->CR = (uint8_t)(state->I >> LITTON_WORD_BITS);
        state->I &= LITTON_WORD_MASK;
        state->I = (state->I << 8) | (state->CR);
        state->CR = (uint8_t)(state->I >> LITTON_WORD_BITS);
        state->I &= LITTON_WORD_MASK;
    }

    /* Return the step result to the caller */
    return result;
}
