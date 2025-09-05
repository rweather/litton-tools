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

#include "tokeniser.h"
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

void litton_tokeniser_init
    (litton_tokeniser_t *tokeniser, FILE *input, const char *filename)
{
    memset(tokeniser, 0, sizeof(litton_tokeniser_t));
    tokeniser->input = input;
    tokeniser->filename = filename;
    tokeniser->line_number = 0;
    tokeniser->token = LTOK_NONE;
}

void litton_tokeniser_free(litton_tokeniser_t *tokeniser)
{
    memset(tokeniser, 0, sizeof(litton_tokeniser_t));
}

int litton_tokeniser_next_line(litton_tokeniser_t *tokeniser)
{
    ++(tokeniser->line_number);
    if (fgets(tokeniser->buf, sizeof(tokeniser->buf), tokeniser->input)) {
        /* Strip whitespace from the end of the line */
        size_t len = strlen(tokeniser->buf);
        while (len > 0 && isspace(tokeniser->buf[len - 1])) {
            --len;
        }

        /* Set up to tokenise the line */
        tokeniser->buf[len] = '\0';
        tokeniser->posn = 0;
        tokeniser->token = LTOK_NONE;
        return 1;
    } else {
        tokeniser->token = LTOK_EOL;
        return 0;
    }
}

static int litton_tokeniser_is_ident(int ch)
{
    if (ch >= 'A' && ch <= 'Z') {
        return 1;
    } else if (ch >= 'a' && ch <= 'z') {
        return 1;
    } else if (ch >= '0' && ch <= '9') {
        return 1;
    } else if (ch == '_' || ch == '.') {
        return 1;
    } else {
        return 0;
    }
}

static int litton_tokeniser_get_digit(char ch, int base)
{
    int digit;
    if (ch >= '0' && ch <= '9') {
        digit = ch - '0';
    } else if (ch >= 'A' && ch <= 'F') {
        digit = ch - 'A' + 10;
    } else if (ch >= 'a' && ch <= 'f') {
        digit = ch - 'a' + 10;
    } else {
        /* Not a digit */
        return -1;
    }
    if (digit >= base) {
        /* Invalid digit for base */
        return -2;
    }
    return digit;
}

static int64_t litton_tokeniser_parse_number
    (litton_tokeniser_t *tokeniser, int base)
{
    char *buf = tokeniser->buf;
    size_t posn = tokeniser->posn;
    int is_neg = 0;
    int64_t value;
    int digit;
    if (buf[posn] == '-') {
        is_neg = 1;
        ++posn;
    }
    digit = litton_tokeniser_get_digit(buf[posn], base);
    if (digit < 0) {
        tokeniser->token = LTOK_ERROR;
        tokeniser->posn = posn;
        litton_error(tokeniser, "invalid digit '%c' for base %d",
                     buf[posn], base);
        return 0;
    }
    value = digit;
    ++posn;
    while ((digit = litton_tokeniser_get_digit(buf[posn], base)) >= 0) {
        value = value * base + digit;
        ++posn;
    }
    if (digit == -2) {
        tokeniser->token = LTOK_ERROR;
        tokeniser->posn = posn;
        litton_error(tokeniser, "invalid digit '%c' for base %d",
                     buf[posn], base);
        return 0;
    }
    if (is_neg) {
        value = -value;
    }
    tokeniser->posn = posn;
    return value;
}

litton_token_t litton_tokeniser_next_token(litton_tokeniser_t *tokeniser)
{
    char *buf = tokeniser->buf;
    size_t posn = tokeniser->posn;
    size_t len;
    char *end;

    /* If we saw an end of line or error last time, keep reporting
     * that until the caller invokes litton_tokeniser_next_line() */
    if (tokeniser->token == LTOK_EOL || tokeniser->token == LTOK_ERROR) {
        return tokeniser->token;
    }

    /* Skip whitespace before the next token */
    while (buf[posn] != '\0' && isspace(buf[posn])) {
        ++posn;
    }

    /* If we have encountered end of line or a comment, we are done */
    if (buf[posn] == '\0' || buf[posn] == ';') {
        tokeniser->posn = posn;
        tokeniser->token = LTOK_EOL;
        return LTOK_EOL;
    }

    /* Next token is an error until we know otherwise */
    tokeniser->token = LTOK_ERROR;

    /* What kind of token do we have? */
    switch (buf[posn]) {
    case 'A': case 'B': case 'C': case 'D': case 'E':
    case 'F': case 'G': case 'H': case 'I': case 'J':
    case 'K': case 'L': case 'M': case 'N': case 'O':
    case 'P': case 'Q': case 'R': case 'S': case 'T':
    case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
    case 'a': case 'b': case 'c': case 'd': case 'e':
    case 'f': case 'g': case 'h': case 'i': case 'j':
    case 'k': case 'l': case 'm': case 'n': case 'o':
    case 'p': case 'q': case 'r': case 's': case 't':
    case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
    case '_': case '.':
        if (posn == 0) {
            /* Identifiers at the start of the line are labels */
            tokeniser->token = LTOK_LABEL;
        } else {
            /* Regular identifier */
            tokeniser->token = LTOK_IDENT;
        }
        tokeniser->name = tokeniser->buf + posn;
        ++posn;
        while (litton_tokeniser_is_ident(buf[posn])) {
            /* Find the end of the identifier */
            ++posn;
        }
        tokeniser->name_len = (tokeniser->buf + posn) - tokeniser->name;
        if (tokeniser->token == LTOK_LABEL && buf[posn] == ':') {
            /* Labels can end in a colon, regular identifiers cannot */
            ++posn;
        }
        break;

    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9': case '-':
        /* Decimal number */
        tokeniser->token = LTOK_NUMBER;
        tokeniser->posn = posn;
        tokeniser->num = litton_tokeniser_parse_number(tokeniser, 10);
        posn = tokeniser->posn;
        break;

    case '%':
        /* Binary number */
        tokeniser->token = LTOK_NUMBER;
        tokeniser->posn = posn + 1;
        tokeniser->num = litton_tokeniser_parse_number(tokeniser, 2);
        posn = tokeniser->posn;
        break;

    case '@':
        /* Octal number */
        tokeniser->token = LTOK_NUMBER;
        tokeniser->posn = posn + 1;
        tokeniser->num = litton_tokeniser_parse_number(tokeniser, 8);
        posn = tokeniser->posn;
        break;

    case '$':
        /* Hexadecimal number */
        tokeniser->token = LTOK_NUMBER;
        tokeniser->posn = posn + 1;
        tokeniser->num = litton_tokeniser_parse_number(tokeniser, 16);
        posn = tokeniser->posn;
        break;

    case '"': case '\'':
        /* String, which extends to the next matching quote or end of line */
        tokeniser->token = LTOK_STRING;
        tokeniser->name = buf + posn + 1;
        end = strchr(buf + posn + 1, buf[posn]);
        if (end) {
            tokeniser->name_len = end - (buf + posn + 1);
            posn = end - buf + 1;
        } else {
            len = strlen(buf);
            tokeniser->name_len = len - (posn + 1);
            posn = len;
        }
        break;

    case '=':
        /* Equals sign, alias for ".equ" */
        tokeniser->token = LTOK_EQUALS;
        ++posn;
        break;

    case ',':
        tokeniser->token = LTOK_COMMA;
        ++posn;
        break;

    default:
        /* Don't know what this is; report an error */
        litton_error(tokeniser, "unexpected character '%c'", buf[posn]);
        break;
    }

    /* Clean up and exit */
    tokeniser->posn = posn;
    return tokeniser->token;
}

void litton_error(litton_tokeniser_t *tokeniser, const char *format, ...)
{
    va_list va;
    fprintf(stderr, "%s:%lu: ", tokeniser->filename, tokeniser->line_number);
    va_start(va, format);
    vfprintf(stderr, format, va);
    va_end(va);
    fputc('\n', stderr);
    ++(tokeniser->num_errors);
}

void litton_error_on_line
    (litton_tokeniser_t *tokeniser, litton_line_number_t line,
     const char *format, ...)
{
    va_list va;
    fprintf(stderr, "%s:%lu: ", tokeniser->filename, line);
    va_start(va, format);
    vfprintf(stderr, format, va);
    va_end(va);
    fputc('\n', stderr);
    ++(tokeniser->num_errors);
}
