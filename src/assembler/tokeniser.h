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

#ifndef LITTON_ASSEM_TOKENISER_H
#define LITTON_ASSEM_TOKENISER_H

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Types of tokens that may be encountered on an assembly source line.
 */
typedef enum
{
    LTOK_NONE,          /**< No token read from the current line yet */
    LTOK_EOL,           /**< End of line */
    LTOK_ERROR,         /**< Unknown token or error */
    LTOK_LABEL,         /**< Identifier that is left-aligned on the line */
    LTOK_IDENT,         /**< Identifier that is not left-aligned */
    LTOK_NUMBER,        /**< Number */
    LTOK_STRING,        /**< String */
    LTOK_EQUALS,        /**< "=" sign */
    LTOK_COMMA          /**< Comma */

} litton_token_t;

/**
 * @brief Type for line numbers in a file being tokenised.
 */
typedef unsigned long litton_line_number_t;

/**
 * @brief Structure for controlling the tokenisation of input assembly files.
 */
typedef struct
{
    /** Current line that is being tokenised */
    char buf[BUFSIZ];

    /** Position in the current line */
    size_t posn;

    /** Number of the current line, for error reporting purposes */
    litton_line_number_t line_number;

    /** Name of the input file for error reporting */
    const char *filename;

    /** Input data stream */
    FILE *input;

    /** Token that was just recognised */
    litton_token_t token;

    /** Pointer to the start of an identifier or string token */
    const char *name;

    /** Length of an identifier or string token */
    size_t name_len;

    /** Value of a number token, positive or negative */
    int64_t num;

    /** Number of errors that have occurred */
    unsigned long num_errors;

} litton_tokeniser_t;

/**
 * @brief Initialises a tokeniser.
 *
 * @param[out] tokeniser The tokeniser to be initialised.
 * @param[in] input Open stdio stream to read from.
 * @param[in] filename Name of the file that is being read from.
 */
void litton_tokeniser_init
    (litton_tokeniser_t *tokeniser, FILE *input, const char *filename);

/**
 * @brief Frees a tokeniser when it is no longer required.
 *
 * @param[in] tokeniser The tokeniser to be freed.
 */
void litton_tokeniser_free(litton_tokeniser_t *tokeniser);

/**
 * @brief Read the next line of input and prepare to tokenise it.
 *
 * @param[in,out] tokeniser The tokeniser.
 *
 * @return Non-zero if a line was read, zero at EOF.
 */
int litton_tokeniser_next_line(litton_tokeniser_t *tokeniser);

/**
 * @brief Read the next token from the current line.
 *
 * @param[in,out] tokeniser The tokeniser.
 *
 * @return The next token.
 */
litton_token_t litton_tokeniser_next_token(litton_tokeniser_t *tokeniser);

/**
 * @brief Reports an error on the current line of input.
 *
 * @param[in,out] tokeniser The tokeniser.
 * @param[in] format The printf-style format for the error message.
 */
void litton_error(litton_tokeniser_t *tokeniser, const char *format, ...);

/**
 * @brief Reports an error on a specific line of input.
 *
 * @param[in,out] tokeniser The tokeniser.
 * @param[in] line The line number.
 * @param[in] format The printf-style format for the error message.
 */
void litton_error_on_line
    (litton_tokeniser_t *tokeniser, litton_line_number_t line,
     const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif
