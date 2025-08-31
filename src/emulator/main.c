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
#include <getopt.h>

static void usage(const char *progname)
{
    fprintf(stderr, "Usage: %s [options] image.drum\n\n", progname);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "    -e ENTRY\n");
    fprintf(stderr, "        Set the entry point to the drum image, in hexadecimal.\n");
    fprintf(stderr, "    -s SIZE\n");
    fprintf(stderr, "        Set the size of the drum, in decimal; default 4096.\n");
    fprintf(stderr, "    -v\n");
    fprintf(stderr, "        Verbose disassembly of instructions as they are executed.\n");
}

static litton_state_t machine;

int main(int argc, char *argv[])
{
    const char *progname = argv[0];
    const char *drum_image;
    litton_step_result_t step;
    int exit_status = 0;
    int opt;

    /* Initialize the machine */
    litton_init(&machine);

    /* Process the command-line options */
    while ((opt = getopt(argc, argv, "e:s:v")) != -1) {
        if (opt == 'e') {
            litton_set_entry_point(&machine, strtoul(optarg, NULL, 16));
        } else if (opt == 's') {
            litton_set_drum_size(&machine, strtoul(optarg, NULL, 0));
        } else if (opt == 'v') {
            machine.disassemble = 1;
        } else {
            usage(progname);
            litton_free(&machine);
            return 1;
        }
    }
    if (optind >= argc) {
        usage(progname);
        litton_free(&machine);
        return 1;
    }
    drum_image = argv[optind];

    /* Load the drum image into memory */
    if (!litton_load_drum(&machine, drum_image)) {
        litton_free(&machine);
    }

    /* Reset the machine */
    litton_reset(&machine);

    /* Keep running the program until halt, illegal instruction, or spinning */
    while ((step = litton_step(&machine)) == LITTON_STEP_OK) {
        /* Keep going */
    }
    switch (step) {
    case LITTON_STEP_OK:
    case LITTON_STEP_HALT:
        /* If the halt code is 0, assume everything is OK.
         * Otherwise report a message and change the exit status. */
        if (machine.register_display != 0) {
            fprintf(stderr, "Halted at address %03X, halt code = %d\n",
                    (unsigned)(machine.PC), machine.register_display);
            exit_status = 1;
        }
        break;

    case LITTON_STEP_ILLEGAL:
        fprintf(stderr, "Illegal instruction at address %03X\n",
                (unsigned)(machine.PC));
        exit_status = 1;
        break;

    case LITTON_STEP_SPINNING:
        fprintf(stderr, "Spinning out of control at address %03X\n",
                (unsigned)(machine.PC));
        exit_status = 1;
        break;
    }
    litton_free(&machine);
    return exit_status;
}
