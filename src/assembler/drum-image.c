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

#include "drum-image.h"
#include <stdlib.h>
#include <string.h>

#define LITTON_DRUM_LOC_MASK (LITTON_DRUM_MAX_SIZE - 1)

void litton_drum_image_init(litton_drum_image_t *drum)
{
    memset(drum, 0, sizeof(litton_drum_image_t));
    drum->entry_point = LITTON_DRUM_MAX_SIZE;
    drum->drum_size = LITTON_DRUM_MAX_SIZE;
    drum->printer_charset = LITTON_CHARSET_ASCII;
    drum->keyboard_charset = LITTON_CHARSET_ASCII;
}

void litton_drum_image_free(litton_drum_image_t *drum)
{
    if (drum->title) {
        free(drum->title);
    }
    memset(drum, 0, sizeof(litton_drum_image_t));
}

static void litton_drum_image_start_word(litton_drum_image_t *drum)
{
    if (drum->posn.posn < drum->drum_size && drum->posn.sub_posn == 0) {
        /* Have we overwritten ourselves? */
        if (drum->used[drum->posn.posn]) {
            drum->overwrite = 1;
        }

        /* Start a new word by filling it with no-ops and set the
         * implicit jump target in the high byte to the next word */
        drum->drum[drum->posn.posn] =
            (((litton_word_t)((drum->posn.posn + 1) & 0x00FF)) << 32) |
            0x0A0A0A0AULL;
        drum->used[drum->posn.posn] = 1;
    } else if (drum->posn.posn >= drum->drum_size) {
        /* Trying to start a new word beyond the end of the drum */
        drum->overflow = 1;
    }
}

static void litton_drum_image_flush_word(litton_drum_image_t *drum)
{
    ++(drum->posn.posn);
    drum->posn.sub_posn = 0;
}

static void litton_drum_image_patch_byte
    (litton_drum_image_t *drum, int sub_posn, uint8_t value)
{
    if (drum->posn.posn < drum->drum_size) {
        if (sub_posn >= 0) {
            litton_word_t mask = 0xFF000000ULL >> (sub_posn * 8);
            drum->drum[drum->posn.posn] =
                (drum->drum[drum->posn.posn] & ~mask) |
                (((litton_word_t)value) << ((3 - sub_posn) * 8));
        } else {
            drum->drum[drum->posn.posn] =
                (drum->drum[drum->posn.posn] & 0x00FFFFFFFFULL) |
                (((litton_word_t)value) << 32);
        }
    } else {
        drum->overflow = 1;
    }
}

static void litton_drum_image_add_byte(litton_drum_image_t *drum, uint8_t value)
{
    litton_drum_image_start_word(drum);
    litton_drum_image_patch_byte(drum, drum->posn.sub_posn, value);
    ++(drum->posn.sub_posn);
}

litton_drum_image_posn_t litton_drum_image_add_insn
    (litton_drum_image_t *drum, uint16_t insn, int resolved)
{
    litton_drum_image_posn_t insn_posn;
    if (insn < 0x0100) {
        /* 8-bit instruction */
        if (drum->posn.sub_posn >= 2 && (drum->posn.posn & 0x00FF) == 0x00FF) {
            /* We are about to cross a page boundary so we need to
             * insert an explicit jump to the next page first. */
            litton_drum_image_align(drum);
        }
        if (drum->posn.sub_posn >= 4) {
            /* Current instruction has already overflowed, so re-align */
            litton_drum_image_align(drum);
        }
        insn_posn = drum->posn;
        litton_drum_image_add_byte(drum, insn);
    } else if ((insn & 0xF000) == LOP_JU && resolved &&
               (insn & 0x0F00) == (drum->posn.posn & 0x0F00)) {
        /* Unconditional jump to the same page.  We may be able to use the
         * implicit jump in the current instruction to do this. */
        insn_posn = drum->posn;
        if (drum->posn.sub_posn >= 3) {
            /* Word is full or there is a single no-op byte left over.
             * Patch the implicit jump in the high byte. */
            litton_drum_image_patch_byte(drum, -1, insn & 0x00FF);
        } else {
            /* There is enough room for a full explicit jump */
            litton_drum_image_add_byte(drum, (insn >> 8) & 0x00FF);
            litton_drum_image_add_byte(drum, insn & 0x00FF);
        }
        litton_drum_image_flush_word(drum);
    } else if ((insn & 0xF000) == LOP_JU) {
        /* Unconditional jump to another page, or the label is unresolved */
        if (drum->posn.sub_posn >= 3) {
            /* Current instruction has already overflowed, so re-align */
            litton_drum_image_align(drum);
        }
        insn_posn = drum->posn;
        litton_drum_image_add_byte(drum, (insn >> 8) & 0x00FF);
        litton_drum_image_add_byte(drum, insn & 0x00FF);
        litton_drum_image_flush_word(drum);
    } else {
        /* 16-bit instruction */
        if (drum->posn.sub_posn >= 1 && (drum->posn.posn & 0x00FF) == 0x00FF) {
            /* We are about to cross a page boundary so we need to
             * insert an explicit jump to the next page first. */
            litton_drum_image_align(drum);
        }
        if (drum->posn.sub_posn >= 3) {
            /* Current instruction has already overflowed, so re-align */
            litton_drum_image_align(drum);
        }
        insn_posn = drum->posn;
        litton_drum_image_add_byte(drum, (insn >> 8) & 0x00FF);
        litton_drum_image_add_byte(drum, insn & 0x00FF);
    }
    return insn_posn;
}

void litton_drum_image_add_word
    (litton_drum_image_t *drum, litton_word_t word)
{
    litton_drum_image_align(drum);
    if (drum->posn.posn < drum->drum_size) {
        /* Have we overwritten ourselves? */
        if (drum->used[drum->posn.posn]) {
            drum->overwrite = 1;
        }

        /* Store the word and advance the drum position */
        drum->drum[drum->posn.posn] = word & LITTON_WORD_MASK;
        drum->used[drum->posn.posn] = 1;
        ++(drum->posn.posn);
    } else {
        /* Trying to start a new word beyond the end of the drum */
        drum->overflow = 1;
    }
}

void litton_drum_image_align(litton_drum_image_t *drum)
{
    /* Bail out if alignment is not needed */
    if (drum->posn.sub_posn == 0) {
        return;
    }

    /* If we have at least 2 spare bytes, use an explicit jump.
     * Otherwise keep any no-ops that are already in the word and
     * let the implicit jump on the instruction word take care of it. */
    if (drum->posn.sub_posn <= 2) {
        uint16_t insn = LOP_JU | ((drum->posn.posn + 1) & LITTON_DRUM_LOC_MASK);
        litton_drum_image_add_byte(drum, (insn >> 8) & 0x00FF);
        litton_drum_image_add_byte(drum, insn & 0x00FF);
    }

    /* Advance to the next drum position */
    litton_drum_image_flush_word(drum);
}

void litton_drum_image_backpatch
    (litton_drum_image_t *drum, litton_drum_image_posn_t posn, uint16_t addr)
{
    litton_word_t word = drum->drum[posn.posn];
    word |= ((litton_word_t)addr) << ((2 - posn.sub_posn) * 8);
    drum->drum[posn.posn] = word;
}

int litton_drum_image_save
    (litton_drum_image_t *drum, const char *filename, const char *title)
{
    FILE *file;
    litton_drum_loc_t addr;
    if ((file = fopen(filename, "w")) == NULL) {
        perror(filename);
        return 0;
    }
    fprintf(file, "#Litton-Drum-Image\n");
    if (title) {
        fprintf(file, "#Title: %s\n", title);
    }
    fprintf(file, "#Drum-Size: %u\n", (unsigned)(drum->drum_size));
    if (drum->entry_point < LITTON_DRUM_MAX_SIZE) {
        fprintf(file, "#Entry-Point: %03X\n", (unsigned)(drum->entry_point));
    }
    if (drum->printer_id != 0) {
        fprintf(file, "#Printer-Character-Set: %s\n",
                litton_charset_to_name(drum->printer_charset));
        fprintf(file, "#Printer-Device: %02X\n", (unsigned)(drum->printer_id));
    }
    if (drum->keyboard_id != 0) {
        fprintf(file, "#Keyboard-Character-Set: %s\n",
                litton_charset_to_name(drum->keyboard_charset));
        fprintf(file, "#Keyboard-Device: %02X\n", (unsigned)(drum->keyboard_id));
    }
    for (addr = 0; addr < drum->drum_size; ++addr) {
        /* Only dump the locations that have been used */
        if (drum->used[addr]) {
            fprintf(file, "%03X:%010LX\n", addr,
                    (unsigned long long)(drum->drum[addr]));
        }
    }
    fclose(file);
    return 1;
}

int litton_drum_image_save_tape
    (litton_drum_image_t *drum, const char *filename)
{
    litton_drum_loc_t addr;
    int need_address = 1;
    int need_slash = 0;
    int need_crlf = 0;
    FILE *file;
    if ((file = fopen(filename, "wb")) == NULL) {
        perror(filename);
        return 0;
    }
    for (addr = 0; addr < drum->drum_size; ++addr) {
        if (drum->used[addr]) {
            /* Used address */
            if (need_slash) {
                putc('/', file);
                need_slash = 0;
            }
            if (need_crlf) {
                putc('\r', file);
                putc('\n', file);
                need_crlf = 0;
            }
            if (need_address) {
                fprintf(file, "%03X#", addr);
                need_address = 0;
            }
            fprintf(file, "%010LX", (unsigned long long)(drum->drum[addr]));
            need_slash = 1;
        } else {
            /* Unused address */
            need_crlf = need_slash;
            need_address = 1;
            need_slash = 0;
        }
    }
    putc(',', file);
    fclose(file);
    return 1;
}

void litton_drum_image_set_title
    (litton_drum_image_t *drum, const char *title, size_t len)
{
    if (drum->title) {
        free(drum->title);
    }
    drum->title = (char *)malloc(len + 1);
    if (!(drum->title)) {
        fputs("out of memory\n", stderr);
        exit(1);
    }
    memcpy(drum->title, title, len);
    drum->title[len] = '\0';
}
