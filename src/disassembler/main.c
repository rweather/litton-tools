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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static litton_state_t machine;
static uint8_t use_mask[LITTON_DRUM_MAX_SIZE];

static void disassemble_raw(void);
static void disassemble_pretty(void);

int main(int argc, char *argv[])
{
    const char *progname = argv[0];
    int exit_status = 0;
    int arg;
    int pretty = 1;

    /* Check for raw or pretty mode */
    if (argc > 1 && !strcmp(argv[1], "--raw")) {
        pretty = 0;
        ++argv;
        --argc;
    }

    /* Need at least one argument */
    if (argc < 2) {
        fprintf(stderr, "Usage: %s [--raw] input.drum ...\n", progname);
        return 1;
    }

    /* Disassemble all of the drum images */
    for (arg = 1; arg < argc; ++arg) {
        litton_init(&machine);
        if (litton_load_drum(&machine, argv[arg], use_mask)) {
            if (argc > 2) {
                printf("\n%s:\n", argv[arg]);
            }
            if (pretty) {
                disassemble_pretty();
            } else {
                disassemble_raw();
            }
        } else {
            exit_status = 1;
        }
        litton_free(&machine);
    }

    /* Done! */
    return exit_status;
}

/* Are all of the bytes valid instructions?  May be a data word if not */
static int is_valid_instruction_word(litton_word_t word)
{
    unsigned posn = 0;
    uint16_t insn;
    int last_nop = 0;
    while (posn < 4) {
        insn = (word >> ((3 - posn) * 8)) & 0xFF;
        ++posn;
        if (insn >= 0x0040) {
            if (posn >= 4) {
                /* Not enough bytes for a two-byte instruction */
                return 0;
            }
            insn <<= 8;
            insn |= (word >> ((3 - posn) * 8)) & 0xFF;
            ++posn;
        }
        if (litton_opcode_by_number(insn) == 0) {
            /* Not a valid instruction */
            return 0;
        }
        if (insn == LOP_NN) {
            last_nop = 1;
        } else if (last_nop) {
            /* Ordinary instruction after no-op, so probably this is data */
            return 0;
        }
    }
    return 1;
}

/* Disassemble the instructions in a word in raw mode */
static void disassemble_word_raw(litton_drum_loc_t addr, litton_word_t word)
{
    const litton_opcode_info_t *opcode;
    unsigned posn = 0;
    unsigned num_insns = 0;
    uint16_t insn;
    while (posn < 4) {
        insn = (word >> ((3 - posn) * 8)) & 0xFF;
        ++posn;
        if (insn >= 0x0040) {
            insn <<= 8;
            insn |= (word >> ((3 - posn) * 8)) & 0xFF;
            ++posn;
        }
        opcode = litton_opcode_by_number(insn);
        if (strlen(opcode->name) < 5) {
            printf("| %-4s ", opcode->name);
        } else {
            printf("| %-5s", opcode->name);
        }
        switch (opcode->operand_type) {
        case LITTON_OPERAND_NONE:
            printf("     ");
            break;

        case LITTON_OPERAND_MEMORY:
            if ((insn & 0xFFF) < 8) {
                /* Probably a scratchpad register address */
                printf("%d    ", insn & 0x07);
            } else {
                printf("$%03X ", insn & 0xFFF);
            }
            break;

        case LITTON_OPERAND_SCRATCHPAD:
        case LITTON_OPERAND_HALT:
            printf("%d    ", insn & 0x07);
            break;

        case LITTON_OPERAND_SHIFT:
            printf("%-3d  ", (insn & 0x7F) + 1);
            break;

        case LITTON_OPERAND_DEVICE:
        case LITTON_OPERAND_CHAR:
            printf("$%02X  ", insn & 0xFF);
            break;
        }
        ++num_insns;
    }
    while (num_insns < 4) {
        printf("|           ");
        ++num_insns;
    }
    addr &= 0x0F00;
    addr |= (word >> 32);
    printf("| NEXT:$%03X\n", addr);
}

static void disassemble_raw(void)
{
    litton_drum_loc_t addr;
    litton_word_t word;
    for (addr = 0; addr < LITTON_DRUM_MAX_SIZE; ++addr) {
        if (!use_mask[addr]) {
            continue;
        }
        word = machine.drum[addr];
        printf("%03X: %02X %02X %02X %02X %02X ", addr,
               (unsigned)((word >> 32) & 0xFF),
               (unsigned)((word >> 24) & 0xFF),
               (unsigned)((word >> 16) & 0xFF),
               (unsigned)((word >> 8) & 0xFF),
               (unsigned)(word & 0xFF));
        if (is_valid_instruction_word(word)) {
            disassemble_word_raw(addr, word);
        } else {
            printf("| DW $%010lX\n", (unsigned long)word);
        }
    }
}

/* Disassemble the instructions in a word in pretty mode */
static void disassemble_word_pretty(litton_drum_loc_t addr, litton_word_t word)
{
    const litton_opcode_info_t *opcode;
    litton_drum_loc_t next_addr = (addr + 1) & 0xFFF;
    unsigned posn = 0;
    uint16_t insn;
    int first = 1;
    while (posn < 4) {
        insn = (word >> ((3 - posn) * 8)) & 0xFF;
        ++posn;
        if (insn >= 0x0040) {
            insn <<= 8;
            insn |= (word >> ((3 - posn) * 8)) & 0xFF;
            ++posn;
        }
        if (insn == LOP_NN) {
            /* Don't bother with no-op's as they are usually padding for
             * when the next instruction doesn't fit in the current word. */
            continue;
        }
        if ((insn & 0xF000) == LOP_JU) {
            /* If we explicitly jump to the next address, then there is
             * no point in dumping the instruction.  It is implicit. */
            if ((insn & 0xFFF) == next_addr) {
                return;
            }
        }
        opcode = litton_opcode_by_number(insn);
        first = 0;
        printf("     %-5s", opcode->name);
        switch (opcode->operand_type) {
        case LITTON_OPERAND_NONE:
            printf("\n");
            break;

        case LITTON_OPERAND_MEMORY:
            if ((insn & 0xFFF) < 8) {
                /* Probably a scratchpad register address */
                printf(" %d\n", insn & 0xFFF);
            } else {
                printf(" $%03X\n", insn & 0xFFF);
            }
            break;

        case LITTON_OPERAND_SCRATCHPAD:
        case LITTON_OPERAND_HALT:
            printf(" %d\n", insn & 0x07);
            break;

        case LITTON_OPERAND_SHIFT:
            printf(" %d\n", (insn & 0x7F) + 1);
            break;

        case LITTON_OPERAND_DEVICE:
        case LITTON_OPERAND_CHAR:
            printf(" $%02X\n", insn & 0xFF);
            break;
        }
        if ((insn & 0xF000) == LOP_JU) {
            /* If we hit an unconditional jump, there's no point
             * disassembling any more instructions from this word. */
            return;
        }
    }
    next_addr = addr & 0x0F00;
    next_addr |= (word >> 32);
    if (first || next_addr != (addr + 1)) {
        /* Not jumping to the next address, so add the implicit jump */
        printf("     %-5s $%03X\n", "JU", next_addr);
    }
}

static int is_alpha_numeric(litton_word_t word)
{
    int bit;
    uint8_t value;
    for (bit = 32; bit >= 0; bit -= 8) {
        value = ((uint8_t)(word >> bit)) & 0x7F;
        if (value >= 0x40) {
            return 0;
        }
    }
    return 1;
}

static void print_alpha_numeric(litton_word_t word)
{
    int bit;
    uint8_t value;
    const char *string_form;
    for (bit = 32; bit >= 0; bit -= 8) {
        value = ((uint8_t)(word >> bit)) & 0x7F;
        litton_char_from_charset
            (value, LITTON_CHARSET_EBS1231, &string_form);
        fputs(string_form, stdout);
    }
}

static void disassemble_pretty(void)
{
    litton_drum_loc_t addr;
    litton_word_t word;
    for (addr = 0; addr < LITTON_DRUM_MAX_SIZE; ++addr) {
        if (!use_mask[addr]) {
            continue;
        }
        word = machine.drum[addr];
        printf("%03X: %02X %02X %02X %02X %02X", addr,
               (unsigned)((word >> 32) & 0xFF),
               (unsigned)((word >> 24) & 0xFF),
               (unsigned)((word >> 16) & 0xFF),
               (unsigned)((word >> 8) & 0xFF),
               (unsigned)(word & 0xFF));
        if (is_alpha_numeric(word)) {
            /* This may be alphanumeric data in the EBC1231 character set */
            printf("    \"");
            print_alpha_numeric(word);
            printf("\"\n");
        } else {
            printf("\n");
        }
        if (is_valid_instruction_word(word)) {
            disassemble_word_pretty(addr, word);
        } else {
            printf("     DW $%010lX\n", (unsigned long)word);
        }
    }
}
