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

litton_opcode_info_t const litton_opcodes[] = {
    {"HH",      LOP_HH,     0x0007,     LITTON_OPERAND_HALT},
    {"AK",      LOP_AK,     0x0000,     LITTON_OPERAND_NONE},
    {"CL",      LOP_CL,     0x0000,     LITTON_OPERAND_NONE},
    {"NN",      LOP_NN,     0x0000,     LITTON_OPERAND_NONE},
    {"CM",      LOP_CM,     0x0000,     LITTON_OPERAND_NONE},
    {"JA",      LOP_JA,     0x0000,     LITTON_OPERAND_NONE},
    {"BI",      LOP_BI,     0x0000,     LITTON_OPERAND_NONE},
    {"SK",      LOP_SK,     0x0000,     LITTON_OPERAND_NONE},
    {"TZ",      LOP_TZ,     0x0000,     LITTON_OPERAND_NONE},
    {"TH",      LOP_TH,     0x0000,     LITTON_OPERAND_NONE},
    {"TN",      LOP_TH,     0x0000,     LITTON_OPERAND_NONE}, /* Alias for TH */
    {"RK",      LOP_RK,     0x0000,     LITTON_OPERAND_NONE},
    {"TP",      LOP_TP,     0x0000,     LITTON_OPERAND_NONE},

    {"LA",      LOP_LA,     0x0007,     LITTON_OPERAND_SCRATCHPAD},
    {"XC",      LOP_XC,     0x0007,     LITTON_OPERAND_SCRATCHPAD},
    {"XT",      LOP_XT,     0x0007,     LITTON_OPERAND_SCRATCHPAD},
    {"TE",      LOP_TE,     0x0007,     LITTON_OPERAND_SCRATCHPAD},
    {"TG",      LOP_TG,     0x0007,     LITTON_OPERAND_SCRATCHPAD},

    {"BLS",     LOP_BLS,    0x007F,     LITTON_OPERAND_SHIFT},
    {"BLSK",    LOP_BLSK,   0x007F,     LITTON_OPERAND_SHIFT},
    {"BLSS",    LOP_BLSS,   0x0000,     LITTON_OPERAND_NONE},
    {"BLSSK",   LOP_BLSSK,  0x0000,     LITTON_OPERAND_NONE},
    {"BLD",     LOP_BLD,    0x007F,     LITTON_OPERAND_SHIFT},
    {"BLDK",    LOP_BLDK,   0x007F,     LITTON_OPERAND_SHIFT},
    {"BLDS",    LOP_BLDS,   0x0000,     LITTON_OPERAND_NONE},
    {"BLDSK",   LOP_BLDSK,  0x0000,     LITTON_OPERAND_NONE},
    {"BRS",     LOP_BRS,    0x007F,     LITTON_OPERAND_SHIFT},
    {"BRSK",    LOP_BRSK,   0x007F,     LITTON_OPERAND_SHIFT},
    {"BRSS",    LOP_BRSS,   0x0000,     LITTON_OPERAND_NONE},
    {"BRSSK",   LOP_BRSSK,  0x0000,     LITTON_OPERAND_NONE},
    {"BRD",     LOP_BRD,    0x007F,     LITTON_OPERAND_SHIFT},
    {"BRDK",    LOP_BRDK,   0x007F,     LITTON_OPERAND_SHIFT},
    {"BRDS",    LOP_BRDS,   0x0000,     LITTON_OPERAND_NONE},
    {"BRDSK",   LOP_BRDSK,  0x0000,     LITTON_OPERAND_NONE},

    {"SI",      LOP_SI,     0x0000,     LITTON_OPERAND_NONE},
    {"RS",      LOP_RS,     0x0000,     LITTON_OPERAND_NONE},
    {"CIO",     LOP_CIO,    0x0000,     LITTON_OPERAND_NONE},
    {"CIE",     LOP_CIE,    0x0000,     LITTON_OPERAND_NONE},
    {"CIOP",    LOP_CIOP,   0x0000,     LITTON_OPERAND_NONE},
    {"CIEP",    LOP_CIEP,   0x0000,     LITTON_OPERAND_NONE},

    {"DLS",     LOP_DLS,    0x007F,     LITTON_OPERAND_SHIFT},
    {"DLSC",    LOP_DLSC,   0x007F,     LITTON_OPERAND_SHIFT},
    {"DLSS",    LOP_DLSS,   0x0000,     LITTON_OPERAND_NONE},
    {"DLSSC",   LOP_DLSSC,  0x0000,     LITTON_OPERAND_NONE},
    {"DLD",     LOP_DLD,    0x007F,     LITTON_OPERAND_SHIFT},
    {"DLDC",    LOP_DLDC,   0x007F,     LITTON_OPERAND_SHIFT},
    {"DLDS",    LOP_DLDS,   0x0000,     LITTON_OPERAND_NONE},
    {"DLDSC",   LOP_DLDSC,  0x0000,     LITTON_OPERAND_NONE},
    {"DRS",     LOP_DRS,    0x007F,     LITTON_OPERAND_SHIFT},
    {"DRD",     LOP_DRD,    0x007F,     LITTON_OPERAND_SHIFT},

    {"OAO",     LOP_OAO,    0x0000,     LITTON_OPERAND_NONE},
    {"OAE",     LOP_OAE,    0x0000,     LITTON_OPERAND_NONE},
    {"OA",      LOP_OA,     0x0000,     LITTON_OPERAND_NONE},
    {"AST",     LOP_AST,    0x0000,     LITTON_OPERAND_NONE},
    {"AS",      LOP_AS,     0x0000,     LITTON_OPERAND_NONE},
    {"OI",      LOP_OI,     0x00FF,     LITTON_OPERAND_CHAR},
    {"IST",     LOP_IST,    0x00FF,     LITTON_OPERAND_DEVICE},
    {"IS",      LOP_IS,     0x00FF,     LITTON_OPERAND_DEVICE},

    {"CA",      LOP_CA,     0x0FFF,     LITTON_OPERAND_MEMORY},
    {"AD",      LOP_AD,     0x0FFF,     LITTON_OPERAND_MEMORY},
    {"ST",      LOP_ST,     0x0FFF,     LITTON_OPERAND_MEMORY},
    {"JM",      LOP_JM,     0x0FFF,     LITTON_OPERAND_MEMORY},
    {"AC",      LOP_AC,     0x0FFF,     LITTON_OPERAND_MEMORY},
    {"JU",      LOP_JU,     0x0FFF,     LITTON_OPERAND_MEMORY},
    {"JC",      LOP_JC,     0x0FFF,     LITTON_OPERAND_MEMORY},

    /* Terminator for the list */
    {0, 0, 0, LITTON_OPERAND_NONE}
};

const litton_opcode_info_t *litton_opcode_by_number(uint16_t insn)
{
    const litton_opcode_info_t *info = litton_opcodes;
    for (; info->name; ++info) {
        if ((insn & ~(info->operand_mask)) == info->opcode) {
            return info;
        }
    }
    return 0;
}

const litton_opcode_info_t *litton_opcode_by_name
    (const char *name, size_t name_len)
{
    const litton_opcode_info_t *info = litton_opcodes;
    for (; info->name; ++info) {
        if (litton_name_match(info->name, name, name_len)) {
            return info;
        }
    }
    return 0;
}

void litton_disassemble_instruction
    (FILE *out, litton_drum_loc_t addr, uint16_t insn)
{
    const litton_opcode_info_t *info = litton_opcode_by_number(insn);
    fprintf(out, "%03X: ", (unsigned)addr);
    if (insn < 0x0100) {
        /* 8-bit instruction */
        fprintf(out, "%02X  ", (unsigned)insn);
    } else {
        /* 16-bit instruction */
        fprintf(out, "%04X", (unsigned)insn);
    }
    if (info) {
        fprintf(out, "   %-6s", info->name);
        insn &= info->operand_mask;
        switch (info->operand_type) {
        case LITTON_OPERAND_NONE: break;

        case LITTON_OPERAND_MEMORY:
            fprintf(out, "$%03X", (unsigned)insn);
            break;

        case LITTON_OPERAND_SCRATCHPAD:
        case LITTON_OPERAND_HALT:
            fprintf(out, "%d", (unsigned)insn);
            break;

        case LITTON_OPERAND_SHIFT:
            fprintf(out, "%d", (unsigned)(insn + 1));
            break;

        case LITTON_OPERAND_DEVICE:
        case LITTON_OPERAND_CHAR:
            fprintf(out, "$%02X", (unsigned)insn);
            break;
        }
    }
    fprintf(out, "\n");
}
