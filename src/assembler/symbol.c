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

#include "symbol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Reference for red-black trees: Algorithms in C++, Robert Sedgwick, 1992 */

void litton_symbol_table_init(litton_symbol_table_t *symbols)
{
    memset(symbols, 0, sizeof(litton_symbol_table_t));
    symbols->nil.left = &(symbols->nil);
    symbols->nil.right = &(symbols->nil);
    symbols->root.right = &(symbols->nil);
}

static void litton_symbol_free
    (litton_symbol_table_t *symbols, litton_symbol_t *symbol)
{
    if (symbol && symbol != &(symbols->nil)) {
        litton_symbol_reference_t *ref = symbol->references;
        litton_symbol_reference_t *next;
        while (ref != 0) {
            next = ref->next;
            free(ref);
            ref = next;
        }
        litton_symbol_free(symbols, symbol->left);
        litton_symbol_free(symbols, symbol->right);
        free(symbol);
    }
}

void litton_symbol_table_free(litton_symbol_table_t *symbols)
{
    litton_symbol_free(symbols, symbols->root.right);
    memset(symbols, 0, sizeof(litton_symbol_table_t));
}

litton_symbol_t *litton_symbol_create
    (litton_symbol_table_t *symbols, const char *name, size_t name_len,
     unsigned long line)
{
    litton_symbol_t *symbol = calloc(1, sizeof(litton_symbol_t) + name_len + 1);
    char *name2;
    if (!symbol) {
        fputs("out of memory\n", stderr);
        exit(1);
    }
    name2 = ((char *)symbol) + sizeof(litton_symbol_t);
    memcpy(name2, name, name_len);
    name2[name_len] = '\0';
    symbol->name = name2;
    symbol->line = line;
    litton_symbol_insert(symbols, symbol);
    return symbol;
}

static int litton_symbol_name_compare_len
    (const char *name1, size_t name1_len, const char *name2)
{
    if (!name1) {
        return name2 ? -1 : 0;
    } else if (!name2) {
        return 1;
    } else {
        size_t name2_len = strlen(name2);
        while (name1_len > 0 && name2_len > 0) {
            int diff = (*name1++) - (*name2++);
            if (diff < 0) {
                return -1;
            } else if (diff > 0) {
                return 1;
            }
            --name1_len;
            --name2_len;
        }
        if (name1_len) {
            return 1;
        } else if (name2_len) {
            return -1;
        } else {
            return 0;
        }
    }
}

static int litton_symbol_name_compare(const char *name1, const char *name2)
{
    if (!name1) {
        return name2 ? -1 : 0;
    } else if (!name2) {
        return 1;
    } else {
        return litton_symbol_name_compare_len(name1, strlen(name1), name2);
    }
}

litton_symbol_t *litton_symbol_lookup_by_name
    (const litton_symbol_table_t *symbols, const char *name, size_t name_len)
{
    litton_symbol_t *symbol = symbols->root.right;
    int cmp;
    while (symbol != &(symbols->nil)) {
        cmp = litton_symbol_name_compare_len(name, name_len, symbol->name);
        if (cmp == 0) {
            return symbol;
        } else if (cmp < 0) {
            symbol = symbol->left;
        } else {
            symbol = symbol->right;
        }
    }
    return 0;
}

/**
 * @brief Rotates the nodes in a red-black tree to rebalance it.
 *
 * @param[in] node The node that is being inserted into the tree.
 * @param[in] y The node to rotate around.
 */
static litton_symbol_t *litton_symbol_rotate
    (litton_symbol_t *node, litton_symbol_t *y)
{
    litton_symbol_t *c;
    litton_symbol_t *gc;
    int cmp, cmp2;
    cmp = litton_symbol_name_compare(node->name, y->name);
    if (cmp < 0) {
        c = y->left;
    } else {
        c = y->right;
    }
    cmp2 = litton_symbol_name_compare(node->name, c->name);
    if (cmp2 < 0) {
        gc = c->left;
        c->left = gc->right;
        gc->right = c;
    } else {
        gc = c->right;
        c->right = gc->left;
        gc->left = c;
    }
    if (cmp < 0) {
        y->left = gc;
    } else {
        y->right = gc;
    }
    return gc;
}

void litton_symbol_insert
    (litton_symbol_table_t *symbols, litton_symbol_t *symbol)
{
    litton_symbol_t *x = &(symbols->root);
    litton_symbol_t *p = x;
    litton_symbol_t *g = x;
    litton_symbol_t *gg = &(symbols->nil);
    int cmp, cmp2;

    /* Prepare the node for insertion */
    symbol->red = 1;
    symbol->left = &(symbols->nil);
    symbol->right = &(symbols->nil);

    /* Find the place to insert the node, rearranging the tree as needed */
    while (x != &(symbols->nil)) {
        gg = g;
        g = p;
        p = x;
        cmp = litton_symbol_name_compare(symbol->name, x->name);
        if (cmp < 0) {
            x = x->left;
        } else {
            x = x->right;
        }
        if (x->left->red && x->right->red) {
            x->red = 1;
            x->left->red = 0;
            x->right->red = 0;
            if (p->red) {
                g->red = 1;
                cmp  = litton_symbol_name_compare(symbol->name, g->name);
                cmp2 = litton_symbol_name_compare(symbol->name, p->name);
                if (cmp != cmp2) {
                    p = litton_symbol_rotate(symbol, g);
                }
                x = litton_symbol_rotate(symbol, gg);
                x->red = 0;
            }
        }
    }

    /* Insert the node into the tree */
    x = symbol;
    cmp = litton_symbol_name_compare(x->name, p->name);
    if (cmp < 0) {
        p->left = x;
    } else {
        p->right = x;
    }
    x->red = 1;
    x->left->red = 0;
    x->right->red = 0;
    if (p->red) {
        g->red = 1;
        cmp  = litton_symbol_name_compare(symbol->name, g->name);
        cmp2 = litton_symbol_name_compare(symbol->name, p->name);
        if (cmp != cmp2) {
            p = litton_symbol_rotate(symbol, g);
        }
        x = litton_symbol_rotate(symbol, gg);
        x->red = 0;
    }
    symbols->root.right->red = 0;
}

void litton_symbol_add_reference
    (litton_symbol_t *symbol, uint32_t address)
{
    litton_symbol_reference_t *ref;
    ref = calloc(1, sizeof(litton_symbol_reference_t));
    if (!ref) {
        fputs("out of memory\n", stderr);
        exit(1);
    }
    ref->address = address;
    ref->next = symbol->references;
    symbol->references = ref;
}
