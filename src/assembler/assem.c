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

#include "assem.h"
#include <string.h>

/* Minimum and maxium values for signed 40-bit Litton words */
#define LITTON_ASSEM_MIN_VALUE (-((int64_t)LITTON_WORD_MSB))
#define LITTON_ASSEM_MAX_VALUE ((int64_t)LITTON_WORD_MASK)

void litton_assem_init
    (litton_assem_t *assem, FILE *input, const char *filename)
{
    litton_tokeniser_init(&(assem->tokeniser), input, filename);
    litton_drum_image_init(&(assem->drum));
    litton_symbol_table_init(&(assem->symbols));
    assem->charset = LITTON_CHARSET_EBS1231;
}

void litton_assem_free(litton_assem_t *assem)
{
    litton_symbol_table_free(&(assem->symbols));
    litton_drum_image_free(&(assem->drum));
    litton_tokeniser_free(&(assem->tokeniser));
}

/**
 * @brief Determine if the next token is a specific assembler directive.
 *
 * @param[in] assem The assembler state.
 * @param[in] name The directive name to look for.
 *
 * @return Non-zero if the next token is @a name; zero otherwise.
 */
static int litton_assem_is_directive(litton_assem_t *assem, const char *name)
{
    if (assem->tokeniser.token != LTOK_IDENT) {
        return 0;
    } else {
        return litton_name_match
            (name, assem->tokeniser.name, assem->tokeniser.name_len);
    }
}

/**
 * @brief Expect an end of line in the input stream.
 *
 * @param[in] assem The assembler state.
 */
static void litton_assem_expect_eol(litton_assem_t *assem)
{
    litton_token_t token = assem->tokeniser.token;
    if (token != LTOK_EOL && token != LTOK_ERROR) {
        litton_error(&(assem->tokeniser), "extra characters on line");
    }
}

/**
 * @brief Handle an escaped character in a string.
 *
 * @param[in] ch The escape code.
 *
 * @return The actual character for the escape.
 */
static int litton_assem_escape_char(int ch)
{
    switch (ch) {
    case 'a': ch = '\a'; break;
    case 'b': ch = '\b'; break;
    case 'f': ch = '\f'; break;
    case 'n': ch = '\n'; break;
    case 'r': ch = '\r'; break;
    case 't': ch = '\t'; break;
    case 'v': ch = '\v'; break;
    default: break;
    }
    return ch;
}

/**
 * @brief Gets the next character in the current string token and converts
 * it into the corresponding character in the current character set.
 *
 * @param[in,out] assem The assembler state.
 * @param[in,out] posn Current position within the string.
 * @param[out] ch Returns the character code.
 *
 * @return Non-zero on success, zero at the end of the string.
 */
static int litton_assem_next_string_char
    (litton_assem_t *assem, size_t *posn, uint8_t *ch)
{
    int nextch;
    char buf[1];
    size_t temp;

    /* Check for the end of the string */
    if (*posn >= assem->tokeniser.name_len) {
        return 0;
    }

    /* Do we have an escape sequence? */
    nextch = assem->tokeniser.name[*posn] & 0xFF;
    if (nextch == '\\' && (*posn + 1) < assem->tokeniser.name_len) {
        /* Deal with C-style escape sequences */
        ++(*posn);
        nextch = assem->tokeniser.name[(*posn)++] & 0xFF;
        nextch = litton_assem_escape_char(nextch);
        buf[0] = (char)nextch;
        temp = 0;
        nextch = litton_char_to_charset(buf, &temp, 1, assem->charset);
    } else {
        /* Convert the character into the output character set */
        nextch = litton_char_to_charset
            (assem->tokeniser.name, posn, assem->tokeniser.name_len,
            assem->charset);
    }
    if (nextch < 0) {
        litton_error
            (&(assem->tokeniser), "invalid character for character set");
        return 0;
    }
    *ch = (uint8_t)nextch;
    return 1;
}

/**
 * @brief Evaluates an expression from the input stream.
 *
 * @param[in,out] assem The assembler state.
 * @param[out] value The value of the expression.
 * @param[in] min_value The minimum allowed value.
 * @param[in] max_value The maximum allowed value.
 *
 * @return Non-zero if an expression was found; zero on error.
 */
static int litton_assem_eval_expr
    (litton_assem_t *assem, int64_t *value, int64_t min_value,
     int64_t max_value)
{
    litton_token_t token = assem->tokeniser.token;
    *value = min_value;
    if (token == LTOK_NUMBER) {
        /* Simple numeric quantity */
        *value = assem->tokeniser.num;
    } else if (token == LTOK_IDENT) {
        /* Look for an already-defined symbol with this name */
        litton_symbol_t *symbol;
        symbol = litton_symbol_lookup_by_name
            (&(assem->symbols), assem->tokeniser.name,
             assem->tokeniser.name_len);
        if (symbol && (symbol->flags & LITTON_SYMBOL_RESOLVED) != 0) {
            /* Already-resolved symbol, so fetch its value */
            *value = symbol->value;
        } else {
            /* Create a forward reference and report an error for it */
            if (!symbol) {
                symbol = litton_symbol_create
                    (&(assem->symbols), assem->tokeniser.name,
                     assem->tokeniser.name_len, assem->tokeniser.line_number);
            }
            litton_error
                (&(assem->tokeniser),
                 "forward reference to '%s' is not allowed", symbol->name);
            return 0;
        }
    } else if (token == LTOK_STRING) {
        /* Single-character string expected */
        size_t posn = 0;
        uint8_t ch = 0;
        if (posn >= assem->tokeniser.name_len) {
            litton_error
                (&(assem->tokeniser), "single character string expected");
            return 0;
        }
        if (litton_assem_next_string_char(assem, &posn, &ch)) {
            *value = ch;
            if (posn < assem->tokeniser.name_len) {
                litton_error
                    (&(assem->tokeniser),
                     "invalid character for character set");
                return 0;
            }
        } else {
            return 0;
        }
        *value = ch;
    } else {
        /* If the token is an error, we have already reported the problem */
        if (token != LTOK_ERROR) {
            litton_error(&(assem->tokeniser), "numeric value expected");
        }
        return 0;
    }
    litton_tokeniser_next_token(&(assem->tokeniser));
    if (*value < min_value || *value > max_value) {
        litton_error
            (&(assem->tokeniser), "value out of range, expecting %Ld to %Ld",
             (long long)min_value, (long long)max_value);
        return 0;
    }
    return 1;
}

/**
 * @brief Parse the operands for an opcode and output it.
 *
 * @param[in,out] assem The assembler state.
 * @param[in] opcode Points to the opcode information.
 */
static void litton_assem_parse_opcode
    (litton_assem_t *assem, const litton_opcode_info_t *opcode)
{
    litton_symbol_t *symbol_forward = 0;
    litton_symbol_t *symbol;
    int ok = 1;
    int resolved = 1;
    int64_t value = 0;
    litton_drum_image_posn_t posn;

    /* Determine what type of operand we need */
    switch (opcode->operand_type) {
    case LITTON_OPERAND_NONE:
        /* No operand */
        break;

    case LITTON_OPERAND_MEMORY:
        /* Need special handling for forward references to identifiers */
        if (assem->tokeniser.token == LTOK_IDENT) {
            /* Look up the symbol or create a new forward reference */
            symbol = litton_symbol_lookup_by_name
                (&(assem->symbols), assem->tokeniser.name,
                 assem->tokeniser.name_len);
            if (!symbol) {
                symbol = litton_symbol_create
                    (&(assem->symbols), assem->tokeniser.name,
                    assem->tokeniser.name_len, assem->tokeniser.line_number);
                symbol->flags |= LITTON_SYMBOL_LABEL;
            }
            if ((symbol->flags & LITTON_SYMBOL_RESOLVED) == 0) {
                /* Skip the token; proceed with 0 as the unresolved address */
                litton_tokeniser_next_token(&(assem->tokeniser));
                value = 0;
                resolved = 0;
                symbol_forward = symbol;
                break;
            }
        }

        /* Literal constants or backwards references must be valid as a
         * memory address for this instruction */
        ok = litton_assem_eval_expr
            (assem, &value, 0, assem->drum.drum_size - 1);
        break;

    case LITTON_OPERAND_SCRATCHPAD:
    case LITTON_OPERAND_HALT:
        /* Scratchpad register number or halt code between 0 and 7 */
        ok = litton_assem_eval_expr(assem, &value, 0, 7);
        break;

    case LITTON_OPERAND_SHIFT:
        /* Shift count between 1 and 128 */
        ok = litton_assem_eval_expr(assem, &value, 1, 128);

        /* Subtract 1 to get the actual operand value to use */
        --value;
        break;

    case LITTON_OPERAND_DEVICE:
    case LITTON_OPERAND_CHAR:
        /* Device selection code or character code between 0 and 255 */
        ok = litton_assem_eval_expr(assem, &value, 0, 255);
        break;
    }
    if (!ok) {
        /* Error while parsing the operand */
        assem->tokeniser.token = LTOK_ERROR;
        return;
    }

    /* Add the instruction to the drum image */
    value &= opcode->operand_mask;
    posn = litton_drum_image_add_insn
        (&(assem->drum), (uint16_t)(opcode->opcode | value), resolved);

    /* Record the forward reference if necessary */
    if (symbol_forward) {
        litton_symbol_add_reference
            (symbol_forward, (((uint32_t)(posn.posn)) << 8) | posn.sub_posn);
    }
}

/**
 * @brief Apply any forward reference fixups for a label.
 *
 * @param[in,out] assem The assembler state.
 * @param[in] label The label that was just defined.
 */
static void litton_assem_apply_fixups
    (litton_assem_t *assem, litton_symbol_t *label)
{
    uint16_t dest = label->value & 0x0FFF;
    litton_symbol_reference_t *ref = label->references;
    while (ref != 0) {
        litton_drum_image_posn_t posn;
        posn.posn = ref->address >> 8;
        posn.sub_posn = ref->address & 0xFF;
        litton_drum_image_backpatch(&(assem->drum), posn, dest);
        ref = ref->next;
    }
}

/**
 * @brief Check all symbols to find any that are still undefined.
 *
 * @param[in,out] assem The assembler state.
 * @param[in] symbol The current symbol to be checked.
 */
static void litton_assem_symbol_check
    (litton_assem_t *assem, litton_symbol_t *symbol)
{
    if (symbol && symbol != &(assem->symbols.nil)) {
        if ((symbol->flags & LITTON_SYMBOL_RESOLVED) == 0) {
            litton_error_on_line
                (&(assem->tokeniser), symbol->line,
                 "'%s' is undefined", symbol->name);
        }
        litton_assem_symbol_check(assem, symbol->left);
        litton_assem_symbol_check(assem, symbol->right);
    }
}

/**
 * @brief Handle the "title" directive: change the title of the drum image.
 *
 * @param[in] assem The assembler state.
 *
 * @return Non-zero on success, zero on error.
 */
static int litton_assem_title(litton_assem_t *assem)
{
    if (assem->drum.title != 0) {
        litton_error(&(assem->tokeniser), "title has already been set");
        return 0;
    }
    if (assem->tokeniser.token != LTOK_STRING) {
        litton_error(&(assem->tokeniser), "title string expected");
        return 0;
    }
    litton_drum_image_set_title
        (&(assem->drum), assem->tokeniser.name, assem->tokeniser.name_len);
    litton_tokeniser_next_token(&(assem->tokeniser));
    return 1;
}

/**
 * @brief Handle the "org" directive: change the origin.
 *
 * @param[in] assem The assembler state.
 *
 * @return Non-zero on success, zero on error.
 */
static int litton_assem_org(litton_assem_t *assem)
{
    int64_t value;

    /* Evaluate the new origin */
    if (!litton_assem_eval_expr(assem, &value, 0, assem->drum.drum_size - 1)) {
        return 0;
    }

    /* Flush out the current in-progress word */
    litton_drum_image_align(&(assem->drum));

    /* Move the origin point */
    assem->drum.posn.posn = (litton_drum_loc_t)value;
    assem->drum.posn.sub_posn = 0;
    return 1;
}

/**
 * @brief Handle the "dw" directive: output a word into the code stream.
 *
 * @param[in] assem The assembler state.
 *
 * @return Non-zero on success, zero on error.
 */
static int litton_assem_dw(litton_assem_t *assem)
{
    int64_t value;
    for (;;) {
        if (assem->tokeniser.token != LTOK_STRING) {
            /* Numeric expression */
            if (!litton_assem_eval_expr
                    (assem, &value, LITTON_ASSEM_MIN_VALUE,
                     LITTON_ASSEM_MAX_VALUE)) {
                return 0;
            }
            litton_drum_image_add_word
                (&(assem->drum), (litton_word_t)(value & LITTON_WORD_MASK));
        } else {
            /* Break the string up into individual characters */
            size_t posn = 0;
            uint8_t ch;
            while (litton_assem_next_string_char(assem, &posn, &ch)) {
                litton_drum_image_add_word(&(assem->drum), ch);
            }
            litton_tokeniser_next_token(&(assem->tokeniser));
        }
        if (assem->tokeniser.token != LTOK_COMMA) {
            break;
        }
        litton_tokeniser_next_token(&(assem->tokeniser));
    }
    return 1;
}

/**
 * @brief Handle the "db" directive: output bytes into the code stream.
 *
 * @param[in] assem The assembler state.
 *
 * @return Non-zero on success, zero on error.
 */
static int litton_assem_db(litton_assem_t *assem)
{
    int64_t value;
    litton_word_t word = 0;
    uint8_t word_posn = 0;
    for (;;) {
        if (assem->tokeniser.token != LTOK_STRING) {
            /* Numeric expression */
            if (!litton_assem_eval_expr(assem, &value, 0, 255)) {
                return 0;
            }
            word |= ((litton_word_t)value) << (32 - word_posn);
            word_posn += 8;
            if (word_posn >= 40) {
                /* Flush the word if it is full */
                litton_drum_image_add_word(&(assem->drum), word);
                word = 0;
                word_posn = 0;
            }
        } else {
            /* Break the string up into individual characters */
            size_t posn = 0;
            uint8_t ch;
            while (litton_assem_next_string_char(assem, &posn, &ch)) {
                word |= ((litton_word_t)ch) << (32 - word_posn);
                word_posn += 8;
                if (word_posn >= 40) {
                    /* Flush the word if it is full */
                    litton_drum_image_add_word(&(assem->drum), word);
                    word = 0;
                    word_posn = 0;
                }
            }
            litton_tokeniser_next_token(&(assem->tokeniser));
        }
        if (assem->tokeniser.token != LTOK_COMMA) {
            break;
        }
        litton_tokeniser_next_token(&(assem->tokeniser));
    }
    if (word_posn != 0) {
        /* Flush the last partial word */
        litton_drum_image_add_word(&(assem->drum), word);
    }
    return 1;
}

/**
 * @brief Handle the "entry" directive: set the drum entry point.
 *
 * @param[in] assem The assembler state.
 *
 * @return Non-zero on success, zero on error.
 */
static int litton_assem_entry(litton_assem_t *assem)
{
    int64_t value;

    /* Error out if we already have an entry point */
    if (assem->drum.entry_point != LITTON_DRUM_MAX_SIZE) {
        litton_error(&(assem->tokeniser), "entry point already set");
        return 0;
    }

    /* Evaluate the entry point expression */
    if (!litton_assem_eval_expr(assem, &value, 0, assem->drum.drum_size - 1)) {
        return 0;
    }

    /* Set the new entry point */
    assem->drum.entry_point = (litton_drum_loc_t)value;
    return 1;
}

/**
 * @brief Parse the name of a character set.
 *
 * @param[in] assem The assembler state.
 * @param[out] charset Returns the character set identifier.
 *
 * @return Non-zero on success, zero on error.
 */
static int litton_assem_parse_charset
    (litton_assem_t *assem, litton_charset_t *charset)
{
    if (assem->tokeniser.token != LTOK_STRING) {
        litton_error(&(assem->tokeniser), "character set name expected");
        return 0;
    }
    if (!litton_charset_from_name
            (charset, assem->tokeniser.name, assem->tokeniser.name_len)) {
        litton_error(&(assem->tokeniser), "unknown character set");
        return 0;
    }
    litton_tokeniser_next_token(&(assem->tokeniser));
    return 1;
}

/**
 * @brief Handle a device directive.
 *
 * @param[in] assem The assembler state.
 * @param[out] id Returns the parsed device identifier.
 * @param[out] charset Returns the parsed device character set.
 * @param[in] symbol_name Symbol to define for the device identifier.
 *
 * @return Non-zero on success, zero on error.
 */
static int litton_assem_device
    (litton_assem_t *assem, uint8_t *id, litton_charset_t *charset,
     const char *symbol_name)
{
    int64_t value;
    litton_symbol_t *symbol;

    /* Error out if we already have a device of this type */
    if (*id != 0) {
        litton_error(&(assem->tokeniser), "%s device is already defined",
                     symbol_name);
        return 0;
    }

    /* Evaluate the device identifier expression */
    if (!litton_assem_eval_expr(assem, &value, 0, 255)) {
        return 0;
    }
    if ((value & 0xF0) == 0 || (value & 0x0F) == 0) {
        /* Must have non-zero bits in the high and low nibbles */
        litton_error(&(assem->tokeniser), "invalid device identifier");
        return 0;
    }

    /* We expect to see a comma next, followed by a character set name */
    if (assem->tokeniser.token != LTOK_COMMA) {
        litton_error(&(assem->tokeniser),
                     "comma expected after device identifier");
        return 0;
    }
    litton_tokeniser_next_token(&(assem->tokeniser));
    if (!litton_assem_parse_charset(assem, charset)) {
        return 0;
    }
    *id = (uint8_t)value;

    /* Set the appropriate symbol to the device identifier so that the
     * program can refer to the device by name later. */
    symbol = litton_symbol_lookup_by_name
        (&(assem->symbols), symbol_name, strlen(symbol_name));
    if (symbol) {
        litton_error
            (&(assem->tokeniser),
             "cannot redefine '%s' as a device identifier", symbol_name);
        return 0;
    }
    symbol = litton_symbol_create
        (&(assem->symbols), symbol_name, strlen(symbol_name),
         assem->tokeniser.line_number);
    symbol->value = *id;
    symbol->flags |= LITTON_SYMBOL_RESOLVED;
    return 1;
}

/**
 * @brief Handle the "printer" directive: set the printer properties.
 *
 * @param[in] assem The assembler state.
 *
 * @return Non-zero on success, zero on error.
 */
static int litton_assem_printer(litton_assem_t *assem)
{
    if (litton_assem_device
            (assem, &(assem->drum.printer_id),
             &(assem->drum.printer_charset), "printer")) {
        /* Also set the active character set based on the printer */
        assem->charset = assem->drum.printer_charset;
        return 1;
    } else {
        return 0;
    }
}

/**
 * @brief Handle the "keyboard" directive: set the keyboard properties.
 *
 * @param[in] assem The assembler state.
 *
 * @return Non-zero on success, zero on error.
 */
static int litton_assem_keyboard(litton_assem_t *assem)
{
    return litton_assem_device
        (assem, &(assem->drum.keyboard_id),
         &(assem->drum.keyboard_charset), "keyboard");
}

/**
 * @brief Handle the "charset" directive: change the charset for strings.
 *
 * @param[in] assem The assembler state.
 *
 * @return Non-zero on success, zero on error.
 */
static int litton_assem_charset(litton_assem_t *assem)
{
    return litton_assem_parse_charset(assem, &(assem->charset));
}

/**
 * @brief Handle the "align" directive: force alignment on the next word.
 *
 * @param[in] assem The assembler state.
 *
 * @return Non-zero on success, zero on error.
 */
static int litton_assem_align(litton_assem_t *assem)
{
    litton_drum_image_align(&(assem->drum));
    return 1;
}

/**
 * @brief Handle the "drumsize" directive: set the size of the drum.
 *
 * @param[in] assem The assembler state.
 *
 * @return Non-zero on success, zero on error.
 */
static int litton_assem_drumsize(litton_assem_t *assem)
{
    int64_t value;
    if (!litton_assem_eval_expr(assem, &value, 256, LITTON_DRUM_MAX_SIZE)) {
        return 0;
    }
    assem->drum.drum_size = (litton_drum_loc_t)value;
    return 1;
}

/**
 * @brief Handle the "isw" psuedo-opcode: select I/O device and wait.
 *
 * @param[in] assem The assembler state.
 *
 * @return Non-zero on success, zero on error.
 */
static int litton_assem_isw(litton_assem_t *assem)
{
    int64_t value;
    litton_word_t word;

    /* Evaluate the device identifier expression */
    if (!litton_assem_eval_expr(assem, &value, 0, 255)) {
        return 0;
    }
    if ((value & 0xF0) == 0 || (value & 0x0F) == 0) {
        /* Must have non-zero bits in the high and low nibbles */
        litton_error(&(assem->tokeniser), "invalid device identifier");
        return 0;
    }

    /* Insert the equivalent of the following code:
     *
     *      label1:
     *          ist device_id
     *          jc label2
     *          ju label1
     *      label2:
     *
     * If we align the drum first, then "ju label1" can be done implicitly.
     */
    litton_drum_image_align(&(assem->drum));
    word = ((litton_word_t)(assem->drum.posn.posn & 0xFF)) << 32;
    word |= ((litton_word_t)(LOP_IST | value)) << 16;
    word |= ((litton_word_t)(LOP_JC | (assem->drum.posn.posn + 1)));
    litton_drum_image_add_word(&(assem->drum), word);
    return 1;
}

/**
 * @brief Handle the output and wait psuedo-opcodes.
 *
 * @param[in] assem The assembler state.
 * @param[in] opcode The opcode to use.
 *
 * @return Non-zero on success, zero on error.
 */
static int litton_assem_oa_wait(litton_assem_t *assem, uint16_t opcode)
{
    litton_word_t word;

    /* Insert the equivalent of the following code:
     *
     *      label1:
     *          oa          ; or oao or oae
     *          jc label2
     *          ju label1
     *      label2:
     *
     * If we align the drum first, then "ju label1" can be done implicitly.
     */
    litton_drum_image_align(&(assem->drum));
    word = ((litton_word_t)(assem->drum.posn.posn & 0xFF)) << 32;
    word |= ((litton_word_t)opcode) << 16;
    word |= ((litton_word_t)(LOP_JC | (assem->drum.posn.posn + 1)));
    litton_drum_image_add_word(&(assem->drum), word);
    return 1;
}

/**
 * @brief Handle the "oaow" psuedo-opcode: output accumulator with odd
 * parity and wait.
 *
 * @param[in] assem The assembler state.
 *
 * @return Non-zero on success, zero on error.
 */
static int litton_assem_oaow(litton_assem_t *assem)
{
    return litton_assem_oa_wait(assem, LOP_OA);
}

/**
 * @brief Handle the "oaew" psuedo-opcode: output accumulator with even
 * parity and wait.
 *
 * @param[in] assem The assembler state.
 *
 * @return Non-zero on success, zero on error.
 */
static int litton_assem_oaew(litton_assem_t *assem)
{
    return litton_assem_oa_wait(assem, LOP_OAE);
}

/**
 * @brief Handle the "oaw" psuedo-opcode: output accumulator and wait.
 *
 * @param[in] assem The assembler state.
 *
 * @return Non-zero on success, zero on error.
 */
static int litton_assem_oaw(litton_assem_t *assem)
{
    return litton_assem_oa_wait(assem, LOP_OA);
}

/**
 * @brief Handle the output immediate and wait psuedo-opcodes.
 *
 * @param[in] assem The assembler state.
 * @param[in] parity The parity to use.
 *
 * @return Non-zero on success, zero on error.
 */
static int litton_assem_oi_wait(litton_assem_t *assem, litton_parity_t parity)
{
    int64_t value;
    litton_word_t word;

    /* Evaluate the character to be output */
    if (!litton_assem_eval_expr(assem, &value, 0, 255)) {
        return 0;
    }

    /* Insert the equivalent of the following code:
     *
     *      label1:
     *          oi char
     *          jc label2
     *          ju label1
     *      label2:
     *
     * If we align the drum first, then "ju label1" can be done implicitly.
     */
    value = litton_add_parity(value, parity);
    litton_drum_image_align(&(assem->drum));
    word = ((litton_word_t)(assem->drum.posn.posn & 0xFF)) << 32;
    word |= ((litton_word_t)(LOP_OI | value)) << 16;
    word |= ((litton_word_t)(LOP_JC | (assem->drum.posn.posn + 1)));
    litton_drum_image_add_word(&(assem->drum), word);
    return 1;
}

/**
 * @brief Handle the "oiow" psuedo-opcode: output immediate with odd
 * parity and wait.
 *
 * @param[in] assem The assembler state.
 *
 * @return Non-zero on success, zero on error.
 */
static int litton_assem_oiow(litton_assem_t *assem)
{
    return litton_assem_oi_wait(assem, LITTON_PARITY_ODD);
}

/**
 * @brief Handle the "oiew" psuedo-opcode: output immediate with even
 * parity and wait.
 *
 * @param[in] assem The assembler state.
 *
 * @return Non-zero on success, zero on error.
 */
static int litton_assem_oiew(litton_assem_t *assem)
{
    return litton_assem_oi_wait(assem, LITTON_PARITY_EVEN);
}

/**
 * @brief Handle the "oiw" psuedo-opcode: output immediate and wait.
 *
 * @param[in] assem The assembler state.
 *
 * @return Non-zero on success, zero on error.
 */
static int litton_assem_oiw(litton_assem_t *assem)
{
    return litton_assem_oi_wait(assem, LITTON_PARITY_NONE);
}

/**
 * @brief Information about a pseudo opcode or directive.
 */
typedef struct
{
    /** Name of the pseudo opcode / directive, NULL for the list terminator */
    const char *name;

    /** Parses and handles the pseudo opcode / directive */
    int (*parse)(litton_assem_t *assem);

} litton_pseudo_opcode_t;

/** List of all pseudo opcodes / directives */
static litton_pseudo_opcode_t const litton_pseudo_opcodes[] = {
    /* Directives */
    {"title",       litton_assem_title},
    {"org",         litton_assem_org},
    {"dw",          litton_assem_dw},
    {"db",          litton_assem_db},
    {"entry",       litton_assem_entry},
    {"printer",     litton_assem_printer},
    {"keyboard",    litton_assem_keyboard},
    {"charset",     litton_assem_charset},
    {"align",       litton_assem_align},
    {"drumsize",    litton_assem_drumsize},

    /* Pseudo opcodes */
    {"isw",         litton_assem_isw},
    {"oaow",        litton_assem_oaow},
    {"oaew",        litton_assem_oaew},
    {"oaw",         litton_assem_oaw},
    {"oiow",        litton_assem_oiow},
    {"oiew",        litton_assem_oiew},
    {"oiw",         litton_assem_oiw},

    /* List terminator */
    {0,             0}
};

/**
 * @brief Looks up a pseudo opcode or directive by name.
 *
 * @param[in] name Points to the name.
 * @param[in] name_len Length of the name.
 *
 * @return Information about the pseudo opcode / directive; or NULL
 * if not found.
 */
static const litton_pseudo_opcode_t *litton_assem_lookup_pseudo
    (const char *name, size_t name_len)
{
    const litton_pseudo_opcode_t *pseudo = litton_pseudo_opcodes;
    while (pseudo->name) {
        if (litton_name_match(pseudo->name, name, name_len)) {
            return pseudo;
        }
        ++pseudo;
    }
    return 0;
}

void litton_assem_parse(litton_assem_t *assem)
{
    litton_token_t token;
    litton_symbol_t *label;
    const litton_opcode_info_t *opcode;
    const litton_pseudo_opcode_t *pseudo;

    for (;;) {
        /* Check for overflow or overwrite on the drum */
        if (assem->drum.overflow) {
            litton_error(&(assem->tokeniser), "drum size exceeded");
            break;
        }
        if (assem->drum.overwrite) {
            litton_error
                (&(assem->tokeniser), "existing code has been overwritten");
            break;
        }

        /* Read the next line from the input */
        if (!litton_tokeniser_next_line(&(assem->tokeniser))) {
            break;
        }

        /* Read the first token on the line */
        token = litton_tokeniser_next_token(&(assem->tokeniser));

        /* Does the line start with a label? */
        if (token == LTOK_LABEL) {
            /* Look up the label in the symbol table */
            label = litton_symbol_lookup_by_name
                (&(assem->symbols), assem->tokeniser.name,
                 assem->tokeniser.name_len);
            if (label) {
                /* Label already exists.  Is it resolved yet? */
                if ((label->flags & LITTON_SYMBOL_RESOLVED) != 0) {
                    /* Yes it is, so this is an attempted redefinition */
                    litton_error
                        (&(assem->tokeniser),
                         "'%s' redefined, previous definition on line %lu",
                         label->name, label->line);
                    continue;
                }

                /* Set the line number for the label's actual definition */
                label->line = assem->tokeniser.line_number;
            } else {
                /* Create a new unresolved label with this name */
                label = litton_symbol_create
                    (&(assem->symbols), assem->tokeniser.name,
                     assem->tokeniser.name_len, assem->tokeniser.line_number);
            }

            /* Skip the label token */
            token = litton_tokeniser_next_token(&(assem->tokeniser));

            /* Is the label followed by "=" or "equ"? */
            if (token == LTOK_EQUALS ||
                    litton_assem_is_directive(assem, "equ")) {
                litton_tokeniser_next_token(&(assem->tokeniser));
                if (litton_assem_eval_expr
                        (assem, &(label->value), LITTON_ASSEM_MIN_VALUE,
                         LITTON_ASSEM_MAX_VALUE)) {
                    label->flags |= LITTON_SYMBOL_RESOLVED;
                } else {
                    /* Error has already been reported, suppress any more */
                    assem->tokeniser.token = LTOK_ERROR;
                }
                litton_assem_expect_eol(assem);
                continue;
            }

            /* This is a code label definition.  Align the code on the
             * next instruction word and record the label's location. */
            litton_drum_image_align(&(assem->drum));
            label->value = assem->drum.posn.posn;
            label->flags |= LITTON_SYMBOL_RESOLVED | LITTON_SYMBOL_LABEL;
            label->line = assem->tokeniser.line_number;

            /* Apply fixups to any previous forward references */
            litton_assem_apply_fixups(assem, label);
        }

        /* We're done if we have end of line or an error at this point */
        if (token == LTOK_EOL || token == LTOK_ERROR) {
            continue;
        }

        /* We now expect to see an opcode or directive */
        if (token != LTOK_IDENT) {
            litton_error(&(assem->tokeniser), "opcode or directive expected");
            continue;
        }
        opcode = litton_opcode_by_name
            (assem->tokeniser.name, assem->tokeniser.name_len);
        if (opcode) {
            /* Core instruction opcode */
            litton_tokeniser_next_token(&(assem->tokeniser));
            litton_assem_parse_opcode(assem, opcode);
        } else {
            /* Directive or pseudo instruction opcode */
            pseudo = litton_assem_lookup_pseudo
                (assem->tokeniser.name, assem->tokeniser.name_len);
            if (pseudo) {
                litton_tokeniser_next_token(&(assem->tokeniser));
                if (!(*(pseudo->parse))(assem)) {
                    /* We have seen one error, suppress any others */
                    assem->tokeniser.token = LTOK_ERROR;
                }
            } else {
                litton_error
                    (&(assem->tokeniser), "unknown opcode or directive");
            }
        }

        /* End of line is now expected */
        litton_assem_expect_eol(assem);
    }

    /* Look for any symbols that are still undefined */
    litton_assem_symbol_check(assem, assem->symbols.root.right);
}
