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

#ifndef LITTON_H
#define LITTON_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Representation of a 40-bit word in the Litton, packed into the
 * low bits of a 64-bit word.
 */
typedef uint64_t litton_word_t;

/**
 * @brief Mask to convert a 64-bit value back into 40-bit.
 */
#define LITTON_WORD_MASK 0x000000FFFFFFFFFFULL

/*----------------------------------------------------------------------*/

/*
 * Information about the magnetic drum memory.
 *
 * Reference: Litton 1600 Technical Reference Manual, section 1.3, "Memory".
 *
 * The drum contains 32 tracks of 128 sectors.  Each sector contains a
 * single 40-bit word.
 *
 * Tracks 30 and 31 are "sealed"; i.e. read-only.  They contain the
 * "OPUS" program.
 *
 * Sectors 0 to 7 on track 0 are reserved.  When accessed by user code,
 * they instead access the sectors in the "scratchpad loop" track
 * (section 1.3.1).
 *
 * The "scratchpad loop" (section 1.3.2) is a special track with a
 * recirculating loop of 8 word values.  The values are aligned so that
 * word i (0..7) of the scratchpad loop is available for quick access
 * when any regular sector with the low 3 bits set to i is being accessed.
 *
 * The "block interchange loop" (section 1.3.3) is a recirculating loop of
 * 8 word values that are accessed as a block.
 *
 * There are also three master timing tracks Z1, Z2, and Z3, and some
 * spare tracks to recover from drum failures on other tracks.
 */

/**
 * @brief Number of tracks on the drum.
 *
 * Tracks are numbered from 0 to 31.  The reference manual refers to
 * these as the 1st and 32nd tracks.
 */
#define LITTON_DRUM_NUM_TRACKS 32

/**
 * @brief Number of sectors on each track of the drum, which are
 * numbered 0 to 127.
 *
 * This is also the number of words per track as there is one word per sector.
 */
#define LITTON_DRUM_NUM_SECTORS 128

/**
 * @brief Number of the first read-only sealed track containing "OPUS".
 */
#define LITTON_DRUM_SEALED_TRACK_1 30

/**
 * @brief Number of the second read-only sealed track containing "OPUS".
 */
#define LITTON_DRUM_SEALED_TRACK_2 31

/**
 * @brief Number of reserved sectors on track 0 of the drum that make up the
 * "scratchpad loop".
 */
#define LITTON_DRUM_RESERVED_SECTORS 8

/**
 * @brief Representation of a 12-bit location on the drum, consisting of a
 * 5-bit track number and a 7-bit sector number.
 */
typedef uint16_t litton_drum_loc_t;

#define litton_loc_create(track, sector) \
    ((litton_drum_loc_t)((((track) & 0x1FU) << 5) | ((sector) & 0x7FU)))

/**
 * @brief Extracts the track number from a drum location.
 *
 * @param[in] location Location on the drum.
 *
 * @return The track number between 0 and LITTON_DRUM_NUM_TRACKS - 1.
 */
#define litton_loc_get_track_number(location) (((location) >> 7) & 0x1FU)

/**
 * @brief Extracts the sector number from a drum location.
 *
 * @param[in] location Location on the drum.
 *
 * @return The sector number between 0 and LITTON_DRUM_NUM_SECTORS - 1.
 */
#define litton_loc_get_sector_number(location) ((location) & 0x7FU)

/*----------------------------------------------------------------------*/

/*
 * Information about Litton commands.
 *
 * Reference: Litton 1600 Technical Reference Manual, section 3.7, "Commands".
 *
 * Commands may be either 8 bits or 16 bits in size.  During execution,
 * the high byte of a 16-bit command will be held in CR, with the low byte
 * in the high order bits of I.
 *
 * Program words are loaded into the 40-bit instruction register (I).
 * They are shifted 8 bits at a time into the command register (CR)
 * for execution.  Once all commands in the instruction register have been
 * used up, a jump is executed to the next word to be executed.
 *
 * Instructions may have one of the following forms:
 *
 *      AAAA 1111 1111 2222 2222
 *      AAAA 1111 1111 2222 3333
 *      AAAA 1111 2222 2222 3333
 *      AAAA 1111 2222 3333 3333
 *      AAAA 1111 2222 3333 4444
 *
 * AAAA is the partial address of the next word to be executed.  1111, 2222,
 * and 3333 are the instructions within the word.
 *
 * At reset time, the unconditional jump 0xEFFF is written to CR and I.
 * This sets CR to 0xEF and I to the contents of address 0xFFF in memory.
 * Consider if the contents of 0xFFF was 0x0158401413, then the sequece of
 * events would be:
 *
 *      CR  I
 *      EF  FF -- -- -- --      Jump to 0xFFF
 *      EF  01 58 40 14 13      Load contents of 0xFFF into I
 *      01  58 40 14 13 EF      Rotate CR and I left
 *      58  40 14 13 EF 01      ditto
 *      40  14 13 EF 01 58      ditto
 *      14  13 EF 01 58 40      ditto
 *      13  EF 01 58 40 14      ditto
 *      EF  01 58 40 14 13      Execute jump to 0xF01
 *
 * Every instruction thus implicitly contains a jump to the next instruction
 * to be executed within the same 256-word page.  To jump to a different page,
 * a full jump instruction needs to appear as 1111, 2222, or 3333.
 *
 * Commands may have a number of different operand types:
 *
 *   M  12-bit memory address consisting of track and sector number
 *   S  3-bit scratchpad sector address (0..7)
 *   N  Shift count - 1 (7-bit)
 *   D  Device select code (8-bit)
 *   C  Character (8-bit)
 *   X  Halt code to display on the register display indicator (3-bit)
 */

/* 8-bit Litton opcodes */
#define LOP_HH      0x00    /** Halt, operand X */
#define LOP_AK      0x08    /** Add K */
#define LOP_CL      0x09    /** Clear A */
#define LOP_NN      0x0A    /** No operation */
#define LOP_CM      0x0B    /** Complement */
#define LOP_JA      0x0D    /** Jump to A */
#define LOP_BI      0x0F    /** Block interchange */
#define LOP_SK      0x10    /** Set K to 1 */
#define LOP_TZ      0x11    /** Test for zero */
#define LOP_TH      0x12    /** Test high order A bit */
#define LOP_TN      0x12    /** Test for negative (alias for TH) */
#define LOP_RK      0x13    /** Reset K to 0 */
#define LOP_TP      0x14    /** Test parity failure */
#define LOP_LA      0x18    /** Logical AND, operand S */
#define LOP_XC      0x20    /** Exchange, operand S */
#define LOP_XT      0x28    /** Extract, operand S */
#define LOP_TE      0x30    /** Test equal, operand S */
#define LOP_TG      0x38    /** Test equal or greater, operand S */

/* 16-bit Litton opcodes */
#define LOP_BLS     0x4000  /** Binary left single shift, operand N */
#define LOP_BLSK    0x4080  /** Binary left single shift incl. K, operand N */
#define LOP_BLSS    0x4100  /** Binary left single shift on scratchpad */
#define LOP_BLSSK   0x4180  /** Binary left single shift on scratchpad, incl. K */
#define LOP_BLD     0x4200  /** Binary left double shift, operand N */
#define LOP_BLDK    0x4280  /** Binary left double shift incl. K, operand N */
#define LOP_BLDS    0x4300  /** Binary left double shift on scratchpad */
#define LOP_BLDSK   0x4380  /** Binary left double shift on scratchpad, incl. K */
#define LOP_BRS     0x4800  /** Binary right single shift, operand N */
#define LOP_BRSK    0x4880  /** Binary right single shift incl. K, operand N */
#define LOP_BRSS    0x4900  /** Binary right single shift on scratchpad */
#define LOP_BRSSK   0x4980  /** Binary right single shift on scratchpad, incl. K */
#define LOP_BRD     0x4A00  /** Binary right double shift, operand N */
#define LOP_BRDK    0x4A80  /** Binary right double shift incl. K, operand N */
#define LOP_BRDS    0x4B00  /** Binary right double shift on scratchpad */
#define LOP_BRDSK   0x4B80  /** Binary right double shift on scratchpad, incl. K */
#define LOP_SI      0x5000  /** Shift input */
#define LOP_RS      0x5080  /** Read status */
#define LOP_CIO     0x5800  /** Clear, input, check odd parity */
#define LOP_CIE     0x5840  /** Clear, input, check even parity */
#define LOP_CIOP    0x5C00  /** Clear, input, check odd parity into A */
#define LOP_CIEP    0x5C40  /** Clear, input, check even parity into A */
#define LOP_DLS     0x6000  /** Decimal left single shift, operand N */
#define LOP_DLSC    0x6080  /** Decimal left single shift plus constant, operand N */
#define LOP_DLSS    0x6100  /** Decimal left single shift on scratchpad */
#define LOP_DLSSC   0x6180  /** Decimal left single shift on scratchpad plus constant */
#define LOP_DLD     0x6200  /** Decimal left double shift, operand N */
#define LOP_DLDC    0x6280  /** Decimal left double shift plus constant, operand N */
#define LOP_DLDS    0x6300  /** Decimal left double shift on scratchpad */
#define LOP_DLDSC   0x6380  /** Decimal left double shift on scratchpad plus constant */
#define LOP_DRS     0x6800  /** Decimal right single shift, operand N */
#define LOP_DRD     0x6A00  /** Decimal right double shift, operand N */
#define LOP_OAO     0x7000  /** Output accumulator with odd parity */
#define LOP_OAE     0x7040  /** Output accumulator with even parity */
#define LOP_OA      0x70C0  /** Output accumulator */
#define LOP_AST     0x74C0  /** Accumulator select on test */
#define LOP_AS      0x76C0  /** Accumulator select */
#define LOP_OI      0x7800  /** Output immediate, operand C */
#define LOP_IST     0x7C00  /** Immediate select on test, operand D */
#define LOP_IS      0x7E00  /** Immediate select, operand D */
#define LOP_CA      0x8000  /** Clear and add / load immediate, operand M */
#define LOP_AD      0x9000  /** Add, operand M */
#define LOP_ST      0xB000  /** Store, operand M */
#define LOP_JM      0xC000  /** Jump mark, operand M */
#define LOP_AC      0xD000  /** Add conditional, operand M */
#define LOP_JU      0xE000  /** Jump unconditional, operand M */
#define LOP_JC      0xF000  /** Jump conditional, operand M */

/*----------------------------------------------------------------------*/

/*
 * State of the Litton machine.
 *
 * Reference: Litton 1600 Technical Reference Manual, various sections.
 */

/**
 * @brief Full state of the Litton machine.
 */
typedef struct
{
    /* Section 1.5, "Registers" */

    /** Command register, 8 bits */
    uint8_t CR;

    /** Buffer register, 8 bits */
    uint8_t B;

    /** Carry register, 1 bit */
    uint8_t K;

    /** Polarity failure register, 1 bit */
    uint8_t P;

    /** Instruction register, 40 bits */
    litton_word_t I;

    /** Accumulator register, 40 bits */
    litton_word_t A;

    /** Contents of drum memory */
    litton_word_t drum[LITTON_DRUM_NUM_TRACKS * LITTON_DRUM_NUM_SECTORS];

    /** Number of instruction cycles that have been executed so far */
    uint64_t cycles;

} litton_state_t;

/**
 * @brief Result of stepping a single instruction.
 */
typedef enum
{
    LITTON_STEP_OK,         /**< Step was OK, execution continues */
    LITTON_STEP_HALT        /**< Processor has halted */

} litton_step_result_t;

/**
 * @brief Initialize the state of the Litton computer.
 *
 * @param[out] state The state to be initialized.
 */
void litton_state_init(litton_state_t *state);

/**
 * @brief Reset the Litton computer.
 *
 * @param[out] state The state of the computer.
 */
void litton_state_reset(litton_state_t *state);

/**
 * @brief Step a single instruction.
 */
litton_step_result_t litton_state_step(litton_state_t *state);

#ifdef __cplusplus
}
#endif

#endif
