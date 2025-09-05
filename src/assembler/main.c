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

#include "assem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

static void usage(const char *progname)
{
    fprintf(stderr, "Usage: %s [options] file.las\n\n", progname);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "    -o OUTPUT\n");
    fprintf(stderr, "        Set the name of the output drum file; default is 'a.drum'.\n");
    fprintf(stderr, "    -t TITLE\n");
    fprintf(stderr, "        Set the title to write to the output drum file.\n");
    fprintf(stderr, "        Overrides the value set by the title directive.\n");
}

int main(int argc, char *argv[])
{
    const char *progname = argv[0];
    const char *output_file = "a.drum";
    const char *source_file;
    const char *title = 0;
    int exit_status = 0;
    int opt;
    FILE *file;
    litton_assem_t assem;

    /* Process the command-line options */
    while ((opt = getopt(argc, argv, "o:t:")) != -1) {
        if (opt == 'o') {
            output_file = optarg;
        } else if (opt == 't') {
            title = optarg;
        } else {
            usage(progname);
            return 1;
        }
    }
    if (optind >= argc) {
        usage(progname);
        return 1;
    }

    /* Open the source file */
    source_file = argv[optind];
    if ((file = fopen(source_file, "r")) == NULL) {
        perror(source_file);
        return 1;
    }

    /* Initialise the assembler */
    litton_assem_init(&assem, file, source_file);

    /* Parse the source file */
    litton_assem_parse(&assem);

    /* Set the default title for the drum image */
    if (!title && assem.drum.title) {
        title = assem.drum.title;
    }

    /* If there were no errors so far, output the drum image */
    if (assem.tokeniser.num_errors == 0) {
        if (!litton_drum_image_save(&assem.drum, output_file, title)) {
            exit_status = 1;
        }
    }

    /* Clean up and exit */
    if (assem.tokeniser.num_errors != 0) {
        exit_status = 1;
    }
    litton_assem_free(&assem);
    fclose(file);
    return exit_status;
}
