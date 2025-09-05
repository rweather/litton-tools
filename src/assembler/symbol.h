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

#ifndef LITTON_ASSEM_SYMBOL_H
#define LITTON_ASSEM_SYMBOL_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Symbol has been resolved to a value */
#define LITTON_SYMBOL_RESOLVED  0x0001
/** Symbol corresponds to a memory label rather than a numeric expression */
#define LITTON_SYMBOL_LABEL     0x0002

/**
 * @brief Information about a forward reference that needs to be patched.
 */
typedef struct litton_symbol_reference_s litton_symbol_reference_t;
struct litton_symbol_reference_s
{
    /** Address in the program that has the forward reference to this symbol */
    uint32_t address;

    /** Next forward reference to this symbol */
    litton_symbol_reference_t *next;
};

/**
 * @brief Information about a symbol.
 */
typedef struct litton_symbol_s litton_symbol_t;
struct litton_symbol_s
{
    /** Name of the symbol */
    char *name;

    /** Value of the symbol if it has been resolved */
    int64_t value;

    /** Non-zero if this node is "red" in the name lookup red-black tree */
    unsigned char red;

    /** Extra flags for the symbol */
    unsigned short flags;

    /** Line number of the symbol's definition in the source file */
    unsigned long line;

    /** Left sub-tree in the name lookup red-black tree */
    litton_symbol_t *left;

    /** Right sub-tree in the name lookup red-black tree */
    litton_symbol_t *right;

    /** Forward references to this symbol */
    litton_symbol_reference_t *references;
};

/**
 * @brief Table of named symbols.
 */
typedef struct
{
    /** Root of the red-black tree */
    litton_symbol_t root;

    /** Sentinel node that represents a leaf */
    litton_symbol_t nil;

} litton_symbol_table_t;

/**
 * @brief Initialises a symbol table.
 *
 * @param[out] symbols The symbol table to initialise.
 */
void litton_symbol_table_init(litton_symbol_table_t *symbols);

/**
 * @brief Frees a symbol table and all of the contained symbols.
 *
 * @param[in] symbols The symbol table to free.
 */
void litton_symbol_table_free(litton_symbol_table_t *symbols);

/**
 * @brief Creates a new symbol in a symbol table.
 *
 * @param[in] symbols The symbol table.
 * @param[in] name The name of the symbol to create.
 * @param[in] name_len Length of the name.
 * @param[in] line Line number where the symbol was first referred to.
 *
 * @return A pointer to the new symbol.
 *
 * It is up to the caller to ensure that the name does not already
 * exist in the symbol table.
 */
litton_symbol_t *litton_symbol_create
    (litton_symbol_table_t *symbols, const char *name, size_t name_len,
     unsigned long line);

/**
 * @brief Looks up a symbol in a symbol table by name.
 *
 * @param[in] symbols The symbol table.
 * @param[in] name The name of the symbol to look for.
 * @param[in] name_len Length of the name.
 *
 * @return A pointer to the symbol, or NULL if @a name was not found.
 */
litton_symbol_t *litton_symbol_lookup_by_name
    (const litton_symbol_table_t *symbols, const char *name, size_t name_len);

/**
 * @brief Inserts a symbol into a symbol table.
 *
 * @param[in,out] symbols The symbol table.
 * @param[in] symbol The symbol to insert.
 *
 * It is assumed that all fields have been initialised except for
 * "left", "right", and "red"; and that the symbol name does not
 * already exist in the symbol table.
 */
void litton_symbol_insert
    (litton_symbol_table_t *symbols, litton_symbol_t *symbol);

/**
 * @brief Adds a reference from a specific instruction in the program.
 *
 * @param[in] symbol The symbol to add the reference to.
 * @param[in] address The address in the program that contains the reference.
 */
void litton_symbol_add_reference
    (litton_symbol_t *symbol, uint32_t address);

#ifdef __cplusplus
}
#endif

#endif
