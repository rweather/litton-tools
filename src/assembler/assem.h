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

#ifndef LITTON_ASSEM_ASSEM_H
#define LITTON_ASSEM_ASSEM_H

#include "tokeniser.h"
#include "drum-image.h"
#include "symbol.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Assembler state.
 */
typedef struct
{
    /** Tokeniser for the source assembly input */
    litton_tokeniser_t tokeniser;

    /** Drum image */
    litton_drum_image_t drum;

    /** Symbol table */
    litton_symbol_table_t symbols;

    /** Character set for strings in the code that follows */
    litton_charset_t charset;

} litton_assem_t;

/**
 * @brief Initialises the assembler state.
 *
 * @param[out] assem The assembler state.
 * @param[in] input Open stdio stream to read from.
 * @param[in] filename Name of the file that is being read from.
 */
void litton_assem_init
    (litton_assem_t *assem, FILE *input, const char *filename);

/**
 * @brief Frees the assembler state.
 *
 * @param[in] assem The assembler state.
 */
void litton_assem_free(litton_assem_t *assem);

/**
 * @brief Parse the assembly source input file.
 *
 * @param[in,out] assem The assembler state.
 */
void litton_assem_parse(litton_assem_t *assem);

#ifdef __cplusplus
}
#endif

#endif
