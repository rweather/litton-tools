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

#include <litton/litton-hl.h>

litton_hl_opcode_info_t const litton_hl_opcodes[] = {
    {"ERR",     LHOP_ERR,   0x0000,     LITTON_HL_OPERAND_NONE},
    {"XCB",     LHOP_XCB,   0x0000,     LITTON_HL_OPERAND_NONE},
    {"ADD",     LHOP_ADD,   0x0000,     LITTON_HL_OPERAND_NONE},
    {"JR",      LHOP_JR,    0x0000,     LITTON_HL_OPERAND_NONE},
    {"JPS",     LHOP_JPS,   0x0000,     LITTON_HL_OPERAND_NONE},
    {"XCV",     LHOP_XCV,   0x0000,     LITTON_HL_OPERAND_NONE},
    {"SCO",     LHOP_SCO,   0x0000,     LITTON_HL_OPERAND_NONE},
    {"AJ",      LHOP_AJ,    0x0000,     LITTON_HL_OPERAND_NONE},
    {"CLR",     LHOP_CLR,   0x0000,     LITTON_HL_OPERAND_NONE},
    {"NGA",     LHOP_NGA,   0x0000,     LITTON_HL_OPERAND_NONE},
    {"NGB",     LHOP_NGB,   0x0000,     LITTON_HL_OPERAND_NONE},
    {"INA",     LHOP_INA,   0x0000,     LITTON_HL_OPERAND_NONE},
    {"SCI",     LHOP_SCI,   0x0000,     LITTON_HL_OPERAND_NONE},
    {"OPUS",    LHOP_OPUS,  0x0000,     LITTON_HL_OPERAND_NONE},
    {"SKIP",    LHOP_SKIP,  0x0000,     LITTON_HL_OPERAND_NONE},
    {"CALC",    LHOP_CALC,  0x0000,     LITTON_HL_OPERAND_NONE},
    {"DCLR",    LHOP_DCLR,  0x0000,     LITTON_HL_OPERAND_NONE},
    {"DIST",    LHOP_DIST,  0x0000,     LITTON_HL_OPERAND_NONE},
    {"DGET",    LHOP_DGET,  0x0000,     LITTON_HL_OPERAND_NONE},
    {"DPUT",    LHOP_DPUT,  0x0000,     LITTON_HL_OPERAND_NONE},
    {"SCAN",    LHOP_SCAN,  0x0000,     LITTON_HL_OPERAND_NONE},
    {"ALFI",    LHOP_ALFI,  0x0000,     LITTON_HL_OPERAND_NONE},
    {"ALFO",    LHOP_ALFO,  0x0000,     LITTON_HL_OPERAND_NONE},
    {"SGET",    LHOP_SGET,  0x0000,     LITTON_HL_OPERAND_NONE},
    {"SPUT",    LHOP_SPUT,  0x0000,     LITTON_HL_OPERAND_NONE},
    {"CDV",     LHOP_CDV,   0x0000,     LITTON_HL_OPERAND_NONE},
    {"DUPE",    LHOP_DUPE,  0x0000,     LITTON_HL_OPERAND_NONE},
    {"DUPO",    LHOP_DUPO,  0x0000,     LITTON_HL_OPERAND_NONE},
    {"SPEC",    LHOP_SPEC,  0x0000,     LITTON_HL_OPERAND_NONE},
    {"IN",      LHOP_IN,    0x000F,     LITTON_HL_OPERAND_INPUT},
    {"MDV",     LHOP_MDV,   0x001F,     LITTON_HL_OPERAND_STORAGE32},
    {"OUT",     LHOP_OUT,   0x001F,     LITTON_HL_OPERAND_STORAGE32},
    {"ACC",     LHOP_ACC,   0x003F,     LITTON_HL_OPERAND_STORAGE},
    {"BV",      LHOP_BV,    0x003F,     LITTON_HL_OPERAND_STORAGE},
    {"SV",      LHOP_SV,    0x003F,     LITTON_HL_OPERAND_STORAGE},
    {"UV",      LHOP_UV,    0x003F,     LITTON_HL_OPERAND_STORAGE},
    {"SEL",     LHOP_SEL,   0x003F,     LITTON_HL_OPERAND_DEVICE},
    {"DUP",     LHOP_DUP,   0x003F,     LITTON_HL_OPERAND_TAB},
    {"CO",      LHOP_CO,    0x003F,     LITTON_HL_OPERAND_CHAR},
    {"TAB",     LHOP_TAB,   0x003F,     LITTON_HL_OPERAND_TAB},
    {"JMK",     LHOP_JMK,   0x007F,     LITTON_HL_OPERAND_PROGRAM},
    {"JZP",     LHOP_JZP,   0x007F,     LITTON_HL_OPERAND_PROGRAM},
    {"JUP",     LHOP_JUP,   0x007F,     LITTON_HL_OPERAND_PROGRAM},

    /* Terminator for the list */
    {0, 0, 0, LITTON_HL_OPERAND_NONE}
};

const litton_hl_opcode_info_t *litton_hl_opcode_by_number(uint16_t insn)
{
    const litton_hl_opcode_info_t *info = litton_hl_opcodes;
    for (; info->name; ++info) {
        if ((insn & ~(info->operand_mask)) == info->opcode) {
            return info;
        }
    }
    return 0;
}

const litton_hl_opcode_info_t *litton_hl_opcode_by_name
    (const char *name, size_t name_len)
{
    const litton_hl_opcode_info_t *info = litton_hl_opcodes;
    for (; info->name; ++info) {
        if (litton_name_match(info->name, name, name_len)) {
            return info;
        }
    }
    return 0;
}
