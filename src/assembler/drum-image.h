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

#ifndef LITTON_ASSEM_DRUM_IMAGE_H
#define LITTON_ASSEM_DRUM_IMAGE_H

#include <litton/litton.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Position within a drum image, consisting of the word position
 * and the sub-position within the word.
 */
typedef struct
{
    /** Position of the word within the drum */
    litton_drum_loc_t posn;

    /** Subposition within the word: 0 to 3 */
    uint8_t sub_posn;

} litton_drum_image_posn_t;

/**
 * @brief Information about a drum image that is being built by the assembler.
 */
typedef struct
{
    /** Words of drum memory */
    litton_word_t drum[LITTON_DRUM_MAX_SIZE];

    /** Flags that indicate which words of drum memory are in use */
    uint8_t used[LITTON_DRUM_MAX_SIZE];

    /** Identifier for the printer device, or 0 if no printer device set */
    uint8_t printer_id;

    /** Identifier for the printer character set */
    litton_charset_t printer_charset;

    /** Identifier for the keyboard device, or 0 if no keyboard device set */
    uint8_t keyboard_id;

    /** Identifier for the keyboard character set */
    litton_charset_t keyboard_charset;

    /** Entry point to the drum, or LITTON_DRUM_MAX_SIZE if not set */
    litton_drum_loc_t entry_point;

    /** Size of the drum, which may be less than LITTON_DRUM_MAX_SIZE */
    litton_drum_loc_t drum_size;

    /** Current position on the drum that is being filled with instructions */
    litton_drum_image_posn_t posn;

    /** Non-zero if the instruction position moved outside the drum */
    int overflow;

    /** Non-zero if the instruction position was already occupied */
    int overwrite;

    /** Title for the drum image */
    char *title;

} litton_drum_image_t;

/**
 * @brief Initialises a drum image.
 *
 * @param[out] drum The drum image.
 */
void litton_drum_image_init(litton_drum_image_t *drum);

/**
 * @brief Frees a drum image.
 *
 * @param[in] drum The drum image.
 */
void litton_drum_image_free(litton_drum_image_t *drum);

/**
 * @brief Add an instruction to the drum image at the current position.
 *
 * @param[in,out] drum The drum image.
 * @param[in] insn The instruction to add, either 8-bit or 16-bit.
 * @param[in] resolved Non-zero if any labels in the instruction have
 * already been resolved; zero if the instruction has a forward reference.
 *
 * @return The position within the drum where the word was added.
 *
 * Padding may be added to the current word if it is not large enough
 * for the new instruction, so the final position may not be the same
 * as the current position.
 */
litton_drum_image_posn_t litton_drum_image_add_insn
    (litton_drum_image_t *drum, uint16_t insn, int resolved);

/**
 * @brief Add a literal word to the drum image at the current position.
 *
 * @param[in,out] drum The drum image.
 * @param[in] word The word to add.
 */
void litton_drum_image_add_word
    (litton_drum_image_t *drum, litton_word_t word);

/**
 * @brief Aligns the instruction stream on a word boundary, inserting
 * no-op's or jumps to get to the next word.
 *
 * @param[in,out] drum The drum image.
 */
void litton_drum_image_align(litton_drum_image_t *drum);

/**
 * @brief Backpatch a memory instruction.
 *
 * @param[in,out] drum The drum image.
 * @param[in] posn Position in the drum image to backpatch.
 * @param[in] addr Address to patch into the instruction.
 */
void litton_drum_image_backpatch
    (litton_drum_image_t *drum, litton_drum_image_posn_t posn, uint16_t addr);

/**
 * @brief Saves the drum image to file.
 *
 * @param[in,out] drum The drum image.
 * @param[in] filename Name of the file to save to.
 * @param[in] title Title for the drum image.
 *
 * @return Non-zero if the drum image was saved, zero on error.
 */
int litton_drum_image_save
    (litton_drum_image_t *drum, const char *filename, const char *title);

/**
 * @brief Sets the title of the drum image.
 *
 * @param[in,out] drum The drum image.
 * @param[in] title Points to the title to set.
 * @param[in] len Length of the title string.
 */
void litton_drum_image_set_title
    (litton_drum_image_t *drum, const char *title, size_t len);

#ifdef __cplusplus
}
#endif

#endif
