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

#ifndef LITTON_HL_H
#define LITTON_HL_H

/* Definitions for the high-level instruction set in OPUS */

#include "litton.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Number of high-level program (P) registers.
 */
#define LITTON_HL_PROGRAM_REGS_NUM 128

/**
 * @brief Address of the high-level program registers in drum memory.
 */
#define LITTON_HL_PROGRAM_REGS_ADDR 0x300

/**
 * @brief Number of high-level storage (V) registers.
 */
#define LITTON_HL_STORAGE_REGS_NUM 64

/**
 * @brief Address of the high-level storage registers in drum memory.
 */
#define LITTON_HL_STORAGE_REGS_ADDR 0x380

/**
 * @brief Number of high-level distribution (D) registers.
 */
#define LITTON_HL_DIST_REGS_NUM 2000

/**
 * @brief Address of the high-level distribution registers in drum memory.
 */
#define LITTON_HL_DIST_REGS_ADDR 0x800

/* High-level opcodes.  Each instruction is 10 bits in size. */
#define LHOP_ERR    0x000   /* Error exit due to invalid instruction */
#define LHOP_XCB    0x001   /* Exchange A and B */
#define LHOP_ADD    0x002   /* Add B to A */
#define LHOP_JR     0x003   /* Jump Return */
#define LHOP_JPS    0x004   /* Jump Positive */
#define LHOP_XCV    0x005   /* Exchange A and V00 */
#define LHOP_SCO    0x006   /* Single character output */
#define LHOP_AJ     0x007   /* Automatic jump to next instruction word */
#define LHOP_CLR    0x008   /* Clear A to zero */
#define LHOP_NGA    0x009   /* Negate A */
#define LHOP_NGB    0x00A   /* Negate B */
#define LHOP_INA    0x00B   /* Input from ASCII-coded tape */
#define LHOP_SCI    0x00E   /* Single character input */
#define LHOP_OPUS   0x00F   /* Return to OPUS */
#define LHOP_SKIP   0x010   /* Skip field from tape */
#define LHOP_CALC   0x011   /* Calculate */
#define LHOP_DCLR   0x012   /* Clear distribution registers to zero */
#define LHOP_DIST   0x013   /* Distribute */
#define LHOP_DGET   0x014   /* Get the distribution register V07 into A */
#define LHOP_DPUT   0x015   /* Put A into the distribution register V07 */
#define LHOP_SCAN   0x016   /* Scan for a non-zero distribution register */
#define LHOP_ALFI   0x017   /* Alphanumeric input */
#define LHOP_ALFO   0x018   /* Alphanumeric output */
#define LHOP_SGET   0x019   /* Load split distribution register */
#define LHOP_SPUT   0x01A   /* Store split distribution register */
#define LHOP_CDV    0x01B   /* Check digit verification */
#define LHOP_DUPE   0x01C   /* Duplicate with even parity */
#define LHOP_DUPO   0x01D   /* Duplicate with odd parity */
#define LHOP_SPEC   0x01E   /* Output special data */
#define LHOP_IN     0x020   /* Input n digits, operand is 0-11 */
#define LHOP_MDV    0x040   /* Multiply divide, operand is V00-V31 */
#define LHOP_OUT    0x060   /* Output, operand is V00-V31 */
#define LHOP_ACC    0x080   /* Add Vnn to A, operand is V00-V63 */
#define LHOP_BV     0x0C0   /* Bring Vnn into A, operand is V00-V63 */
#define LHOP_SV     0x100   /* Store A into Vnn, operand is V00-V63 */
#define LHOP_UV     0x140   /* Add A to Vnn, operand is V00-V63 */
#define LHOP_SEL    0x180   /* Select device nn, operand is 0-63 */
#define LHOP_DUP    0x1C0   /* Duplicate, operand is encoded 1-190 */
#define LHOP_CO     0x200   /* Literal character output, operand is 0-63 */
#define LHOP_TAB    0x240   /* Tab, operand is encoded 1-190 */
#define LHOP_JMK    0x280   /* Jump Mark, operand is P000-P127 */
#define LHOP_JZP    0x300   /* Jump if zero, operand is P000-P127 */
#define LHOP_JUP    0x380   /* Jump unconditional, operand is P000-P127 */

/**
 * @brief Types of high-level instruction operands.
 */
typedef enum
{
    LITTON_HL_OPERAND_NONE,         /**< No operand */
    LITTON_HL_OPERAND_PROGRAM,      /**< 7-bit program memory address */
    LITTON_HL_OPERAND_STORAGE,      /**< 6-bit storage memory address */
    LITTON_HL_OPERAND_STORAGE32,    /**< 5-bit storage memory address */
    LITTON_HL_OPERAND_CHAR,         /**< Literal character */
    LITTON_HL_OPERAND_INPUT,        /**< Input digit count */
    LITTON_HL_OPERAND_DEVICE,       /**< 6-bit device selection code */
    LITTON_HL_OPERAND_TAB           /**< Encoded tab number, 1-190 */

} litton_hl_operand_type_t;

/**
 * @brief Information about a high-level opcode.
 */
typedef struct
{
    /** Name of the opcode, in upper case */
    const char *name;

    /** 10-bit opcode number */
    uint16_t opcode;

    /** Operand mask; bits other than these are the opcode */
    uint16_t operand_mask;

    /** Type of operand for the opcode */
    litton_hl_operand_type_t operand_type;

} litton_hl_opcode_info_t;

/**
 * @brief List of all known high-level opcodes, terminated by an entry
 * with a NULL name.
 */
extern litton_hl_opcode_info_t const litton_hl_opcodes[];

/**
 * @brief Gets the information about a high-level opcode given the
 * instruction number.
 *
 * @param[in] insn The instruction.
 *
 * @return A pointer to the opcode information or NULL if @a insn does
 * not correspond to a known opcode.
 */
const litton_hl_opcode_info_t *litton_hl_opcode_by_number(uint16_t insn);

/**
 * @brief Gets the information about a hgh-level opcode given its name.
 *
 * @param[in] name Points to the name.
 * @param[in] name_len Length of the name.
 *
 * @return A pointer to the opcode information or NULL if @a name does
 * not correspond to a known opcode name.
 */
const litton_hl_opcode_info_t *litton_hl_opcode_by_name
    (const char *name, size_t name_len);

#ifdef __cplusplus
}
#endif

#endif
