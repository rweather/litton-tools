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
static void disassemble_straighten(void);

int main(int argc, char *argv[])
{
    const char *progname = argv[0];
    int exit_status = 0;
    int arg;
    int pretty = 1;
    int straighten = 0;

    /* Check for raw or pretty mode */
    if (argc > 1 && !strcmp(argv[1], "--raw")) {
        pretty = 0;
        ++argv;
        --argc;
    }
    if (argc > 1 && !strcmp(argv[1], "--pretty")) {
        pretty = 1;
        ++argv;
        --argc;
    }
    if (argc > 1 && !strcmp(argv[1], "--straighten")) {
        straighten = 1;
        pretty = 1;
        ++argv;
        --argc;
    }

    /* Need at least one argument */
    if (argc < 2) {
        fprintf(stderr, "Usage: %s [--raw|--pretty|--straighten] input.drum ...\n", progname);
        fprintf(stderr, "\n");
        fprintf(stderr, "    --raw\n");
        fprintf(stderr, "        Disassemble in raw format.\n\n");
        fprintf(stderr, "    --pretty\n");
        fprintf(stderr, "        Disassemble in pretty format (this is the default).\n\n");
        fprintf(stderr, "    --straighten\n");
        fprintf(stderr, "        Disassemble in pretty format but rearrange the code to\n");
        fprintf(stderr, "        straighten out the flow of control.\n");
        return 1;
    }

    /* Disassemble all of the drum images */
    for (arg = 1; arg < argc; ++arg) {
        litton_init(&machine);
        if (litton_load_drum(&machine, argv[arg], use_mask)) {
            if (argc > 2) {
                printf("\n%s:\n", argv[arg]);
            }
            if (straighten) {
                disassemble_straighten();
            } else if (pretty) {
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
    if ((word & 0xFFFF000000ULL) == 0) {
        /* Instruction starts with $0000, so it may just be a literal */
        return 0;
    }
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
    const char *string_form;
    if (word == 0) {
        return 0;
    }
    for (bit = 32; bit >= 0; bit -= 8) {
        value = ((uint8_t)(word >> bit)) & 0xFF;
        if (value >= 0x40) {
            return 0;
        }
        litton_char_from_charset
            (value, LITTON_CHARSET_EBS1231, &string_form);
        if (string_form[0] == '[' || string_form[0] < 0x20) {
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
            /* This may be alphanumeric data in the EBS1231 character set */
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

static uint8_t visited[LITTON_DRUM_MAX_SIZE];

#define VISIT_NONE 0
#define VISIT_DONE 1
#define VISIT_SUBROUTINE 2
#define VISIT_CONDITIONAL 3
#define VISIT_VARIABLE 4

static void disassemble_visit(litton_drum_loc_t addr)
{
    litton_word_t word;
    litton_drum_loc_t next_addr;
    litton_drum_loc_t other_addr;
    const litton_opcode_info_t *opcode;
    unsigned posn;
    uint16_t insn;
    uint8_t is_var;
    int first;
    for (;;) {
        /* Get the next word and mark it as done */
        word = machine.drum[addr];
        is_var = (visited[addr] == VISIT_VARIABLE);
        visited[addr] = VISIT_DONE;

        /* Dump the address and word in hexadecimal */
        printf("%03X: %02X %02X %02X %02X %02X", addr,
               (unsigned)((word >> 32) & 0xFF),
               (unsigned)((word >> 24) & 0xFF),
               (unsigned)((word >> 16) & 0xFF),
               (unsigned)((word >> 8) & 0xFF),
               (unsigned)(word & 0xFF));
        if (is_alpha_numeric(word)) {
            /* This may be alphanumeric data in the EBS1231 character set */
            printf("  \"");
            print_alpha_numeric(word);
            printf("\"  ");
        } else {
            printf("           ");
        }
        first = 1;

        /* Does the word look like an instruction or data word? */
        if (!is_valid_instruction_word(word) || is_var) {
            /* Data word; dump it as "DW" and we're done */
            printf("DW    $%010lX\n", (unsigned long)word);
            break;
        }

        /* Disassemble the instructions in the word */
        next_addr = LITTON_DRUM_MAX_SIZE;
        posn = 0;
        while (posn < 4) {
            insn = (word >> ((3 - posn) * 8)) & 0xFF;
            ++posn;
            if (insn >= 0x0040 && posn < 4) {
                insn <<= 8;
                    insn |= (word >> ((3 - posn) * 8)) & 0xFF;
                ++posn;
            }
            if (insn == LOP_NN) {
                /* Don't bother with no-op's as they are usually padding for
                 * when the next instruction doesn't fit in the current word. */
                continue;
            } else if (insn == LOP_JA) {
                /* Jump to accumulator is usually "return from subroutine",
                 * so this is the end of the current visit chain. */
                if (first) {
                    printf("%s\n", "JA");
                } else {
                    printf("                              %s\n", "JA");
                }
                return;
            }
            opcode = litton_opcode_by_number(insn);
            if ((insn & 0xF000) == LOP_JU) {
                /* Explicit jump which we treat as the next address.
                 * There is no point disassembing the rest of the word
                 * as the following instructions will never be reached. */
                next_addr = insn & 0xFFF;
                break;
            }
            if ((insn & 0xF000) == LOP_JC && posn >= 4) {
                /*
                 * It is common in I/O code to have a busy wait loop like this:
                 *
                 *   this:
                 *       OI $NN
                 *       JC next
                 *       JU this
                 *   next:
                 *
                 * Recognise this form and treat "next" as the next address
                 * to be visited after this word.  Otherwise we will stop at
                 * "JU this" on the current visit iteration.
                 */
                if ((word >> 32) == (addr & 0xFF) &&
                        visited[insn & 0xFFF] == VISIT_NONE) {
                    next_addr = insn & 0xFFF;
                    if (first) {
                        printf("%-5s", "JC");
                        first = 0;
                    } else {
                        printf("                              %-5s", "JC");
                    }
                    printf(" $%03X\n", insn & 0xFFF);
                    printf("                              %-5s", "JU");
                    printf(" $%03X\n", addr);
                    break;
                }
            }
            if ((insn & 0xF000) == LOP_JC) {
                /* Mark the destination of conditional jumps for
                 * preferential visiting on the next pass as they are
                 * probably still part of the current subroutine. */
                other_addr = insn & 0xFFF;
                if (visited[other_addr] == VISIT_NONE) {
                    visited[other_addr] = VISIT_CONDITIONAL;
                }
            }
            if (first) {
                printf("%-5s", opcode->name);
                first = 0;
            } else {
                printf("                              %-5s", opcode->name);
            }
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
        }

        /* Determine the address of the next instruction to visit.
         * If it has already been visited, jump to it and stop.
         * Otherwise fall through and visit it as the new word. */
        if (next_addr >= LITTON_DRUM_MAX_SIZE) {
            next_addr = (addr & 0x0F00) | (word >> 32);
        }
        if (visited[next_addr] != VISIT_NONE) {
            if (first) {
                printf("%-5s $%03X\n", "JU", next_addr);
            } else {
                printf("                              %-5s $%03X\n",
                       "JU", next_addr);
            }
            break;
        }
        if (first) {
            /* All instructions were elided for this word.  Add an
             * explicit jump to the next word so we have something. */
            printf("%-5s $%03X\n", "JU", next_addr);
        }
        addr = next_addr;
    }
}

static void find_address_using_instructions(litton_word_t word)
{
    unsigned posn = 0;
    uint16_t insn;
    litton_drum_loc_t addr;
    while (posn < 4) {
        insn = (word >> ((3 - posn) * 8)) & 0xFF;
        ++posn;
        if (insn >= 0x0040 && posn < 4) {
            insn <<= 8;
                insn |= (word >> ((3 - posn) * 8)) & 0xFF;
            ++posn;
        }
        addr = insn & 0xFFF;
        switch (insn & 0xF000) {
        case LOP_JM:
            /* Mark the addresses of subroutines */
            if (visited[addr] == VISIT_NONE) {
                visited[addr] = VISIT_SUBROUTINE;
            }
            break;

        case LOP_CA:
        case LOP_AD:
        case LOP_ST:
        case LOP_AC:
            /* Address probably refers to a variable or constant.
             * Force the word to be dumped as a literal. */
            if (visited[addr] == VISIT_NONE) {
                visited[addr] = VISIT_VARIABLE;
            }
            break;
        }
    }
}

static void disassemble_straighten(void)
{
    litton_drum_loc_t addr;

    /* Mark all words as unvisited to begin with */
    memset(visited, VISIT_NONE, sizeof(visited));

    /* Mark anything that is not in the original drum image as done */
    for (addr = 0; addr < LITTON_DRUM_MAX_SIZE; ++addr) {
        if (!use_mask[addr]) {
            visited[addr] = VISIT_DONE;
        }
    }

    /* Find instructions that use an address and classify them */
    for (addr = 0; addr < LITTON_DRUM_MAX_SIZE; ++addr) {
        if (is_valid_instruction_word(machine.drum[addr])) {
            find_address_using_instructions(machine.drum[addr]);
        }
    }

    /* Start by visiting the entry point if we have one */
    if (use_mask[machine.entry_point]) {
        disassemble_visit(machine.entry_point);
    }

    /* Loop continuously until we can't find anything else to visit.
     * In the worst case, this exhibits O(n^2) behaviour.  That is,
     * 4096^2 = 16777216 visits.  Modern computers can handle that. */
    for (;;) {
        /* Try finding a conditional jump destination, as it is probably
         * still part of the last subroutine we were visiting. */
        for (addr = 0; addr < LITTON_DRUM_MAX_SIZE; ++addr) {
            if (visited[addr] == VISIT_CONDITIONAL) {
                break;
            }
        }
        if (addr < LITTON_DRUM_MAX_SIZE) {
            disassemble_visit(addr);
            continue;
        }

        /* Try finding a new subroutine entry point */
        for (addr = 0; addr < LITTON_DRUM_MAX_SIZE; ++addr) {
            if (visited[addr] == VISIT_SUBROUTINE) {
                break;
            }
        }
        if (addr < LITTON_DRUM_MAX_SIZE) {
            printf(";\n;\n;\n"); /* Print a separator before the subroutine */
            disassemble_visit(addr);
            continue;
        }

        /* Find any other address that hasn't been visited yet */
        for (addr = 0; addr < LITTON_DRUM_MAX_SIZE; ++addr) {
            if (visited[addr] == VISIT_NONE ||
                    visited[addr] == VISIT_VARIABLE) {
                break;
            }
        }
        if (addr >= LITTON_DRUM_MAX_SIZE) {
            /* Everything has now been visited */
            break;
        }

        /* Visit the address */
        disassemble_visit(addr);
    }
}
