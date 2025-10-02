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
#include <time.h>

static void usage(const char *progname)
{
    fprintf(stderr, "Usage: %s [options] image.drum\n\n", progname);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "    -f\n");
    fprintf(stderr, "        Fast mode; do not slow down to the original speed.\n");
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
    int fast_mode = 0;
    int exit_status = 0;
    int opt;
    uint64_t elapsed_ns;
    uint64_t checkpoint_counter;
    struct timespec checkpoint_time;
    struct timespec sleep_to_time;
    struct timespec now_time;

    /* Initialize the machine */
    litton_init(&machine);

    /* Process the command-line options */
    while ((opt = getopt(argc, argv, "fe:s:v")) != -1) {
        if (opt == 'e') {
            litton_set_entry_point(&machine, strtoul(optarg, NULL, 16));
        } else if (opt == 'f') {
            fast_mode = 1;
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
    if (!litton_load_drum(&machine, drum_image, NULL)) {
        litton_free(&machine);
        return 1;
    }

    /* Create the standard devices */
    litton_create_default_devices(&machine);
    litton_add_tape_punch
        (&machine, LITTON_DEVICE_PUNCH, LITTON_CHARSET_EBS1231);
    litton_add_tape_reader
        (&machine, LITTON_DEVICE_READER, LITTON_CHARSET_EBS1231);

    /* Reset the machine */
    litton_reset(&machine);

    /* Press HALT, READY, and then RUN to start running the program */
    litton_press_button(&machine, LITTON_BUTTON_HALT);
    litton_press_button(&machine, LITTON_BUTTON_READY);
    litton_press_button(&machine, LITTON_BUTTON_RUN);

    /* Keep running the program until halt, illegal instruction, or spinning */
    checkpoint_counter = machine.cycle_counter;
    clock_gettime(CLOCK_MONOTONIC, &checkpoint_time);
    for (;;) {
        /* Step the next instruction */
        if ((step = litton_step(&machine)) != LITTON_STEP_OK) {
            break;
        }

        /* Simulate the actual speed of the computer */
        if (!fast_mode) {
            elapsed_ns = (machine.cycle_counter - checkpoint_counter) * 1000;
            sleep_to_time = checkpoint_time;
            sleep_to_time.tv_nsec += elapsed_ns % 1000000000;
            sleep_to_time.tv_sec += elapsed_ns / 1000000000;
            while (sleep_to_time.tv_nsec >= 1000000000) {
                sleep_to_time.tv_nsec -= 1000000000;
                ++(sleep_to_time.tv_sec);
            }
            clock_gettime(CLOCK_MONOTONIC, &now_time);
            if (machine.acceleration_counter != 0 ||
                    now_time.tv_sec > sleep_to_time.tv_sec ||
                    (now_time.tv_sec == sleep_to_time.tv_sec &&
                     now_time.tv_nsec >= sleep_to_time.tv_nsec)) {
                /* Deadline has already passed, so resynchronise on "now" */
                checkpoint_counter = machine.cycle_counter;
                checkpoint_time = now_time;
            } else {
                clock_nanosleep
                    (CLOCK_MONOTONIC, TIMER_ABSTIME, &sleep_to_time, NULL);
            }
        }
    }
    switch (step) {
    case LITTON_STEP_OK:
    case LITTON_STEP_HALT:
        /* If the halt code is 0, assume everything is OK.
         * Otherwise report a message and change the exit status. */
        if (machine.halt_code != 0) {
            fprintf(stderr, "Halted at address %03X, halt code = %d\n",
                    (unsigned)(machine.PC), machine.halt_code);
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
