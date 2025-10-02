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
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mutex.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <unistd.h>
#include "images.h"

static void usage(const char *progname)
{
    fprintf(stderr, "Usage: %s [options] [image.drum]\n\n", progname);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "    -m\n");
    fprintf(stderr, "        Start in maximised mode.\n");
    fprintf(stderr, "    -v\n");
    fprintf(stderr, "        Verbose disassembly of instructions as they are executed.\n");
}

/** Maximum number of lines to keep in the printer scroll-back buffer */
#define PRINTER_MAX_LINES 12

/** Maximum size of a printer line before auto-CRLF */
#define PRINTER_LINE_SIZE 200

/** Size of the keyboard input buffer */
#define KEYBOARD_BUFFER_SIZE 16

/* Extra buttons that are unique to this UI */
#define LITTON_BUTTON_DRUM_LOAD 0x10000000U
#define LITTON_BUTTON_DRUM_SAVE 0x20000000U
#define LITTON_BUTTON_TAPE_IN   0x40000000U
#define LITTON_BUTTON_TAPE_OUT  0x80000000U

/**
 * @brief State information for managing the SDL user interface.
 */
typedef struct
{
    /** Set to 1 when the program should quit */
    int quit;

    /** Main SDL window */
    SDL_Window *window;

    /** SDL renderer */
    SDL_Renderer *renderer;

    /** Thread for running the actual machine in the background */
    SDL_Thread *run_thread;

    /** Background image */
    SDL_Texture *image_bg;

    /** Lit lamps to overlay on the background */
    SDL_Texture *image_lamps;

    /** Pressed buttons to overlay on the background */
    SDL_Texture *image_buttons;

    /** Main knob in the control up position */
    SDL_Texture *image_control_up;

    /** Main knob in the control down position */
    SDL_Texture *image_control_down;

    /** Main knob in the A0 position */
    SDL_Texture *image_knob_A0;

    /** Main knob in the A8 position */
    SDL_Texture *image_knob_A8;

    /** Main knob in the A16 position */
    SDL_Texture *image_knob_A16;

    /** Main knob in the A24 position */
    SDL_Texture *image_knob_A24;

    /** Main knob in the A32 position */
    SDL_Texture *image_knob_A32;

    /** Main knob in the I0 position */
    SDL_Texture *image_knob_I0;

    /** Main knob in the I8 position */
    SDL_Texture *image_knob_I8;

    /** Main knob in the I16 position */
    SDL_Texture *image_knob_I16;

    /** Main knob in the I24 position */
    SDL_Texture *image_knob_I24;

    /** Main knob in the I32 position */
    SDL_Texture *image_knob_I32;

    /** Font for displaying the printer output */
    TTF_Font *font;

    /** Mutex lock for co-ordinating with the background thread */
    SDL_mutex *mutex;

    /** Identifier of the button that is currently pressed and held */
    uint32_t pressed_button;

    /** Identifier of the button that is selected but not active yet */
    uint32_t selected_button;

    /** Printer output buffer */
    uint8_t printer_output[PRINTER_MAX_LINES][PRINTER_LINE_SIZE];

    /** Current printer column, between 0 and PRINTER_LINE_SIZE-1 */
    int printer_column;

    /** Current printer line, between 0 and PRINTER_MAX_LINES-1 */
    int printer_line;

    /** Width of a character in the font */
    int font_width;

    /** Height of a line of text in the font */
    int font_height;

    /** Keyboard input buffer */
    uint8_t keyboard_input[KEYBOARD_BUFFER_SIZE];

    /** Number of characters in the keyboard input buffer */
    int keyboard_count;

} litton_ui_state_t;

static litton_state_t machine;
static litton_ui_state_t ui;

static void draw_lamp(uint32_t lamps, uint32_t lamp, int x, int y)
{
    SDL_Rect lamp_rect = {
        .x = x,
        .y = y,
        .w = LAMP_WIDTH,
        .h = LAMP_HEIGHT
    };
    if ((lamps & lamp) != 0) {
        SDL_RenderCopy(ui.renderer, ui.image_lamps, &lamp_rect, &lamp_rect);
    }
}

static void draw_pressed_button(int x, int y)
{
    SDL_Rect button_rect = {
        .x = x,
        .y = y,
        .w = BUTTON_WIDTH,
        .h = BUTTON_HEIGHT
    };
    SDL_RenderCopy(ui.renderer, ui.image_buttons, &button_rect, &button_rect);
}

static void draw_pressed_button_sized(int x, int y, int w, int h)
{
    SDL_Rect button_rect = {
        .x = x,
        .y = y,
        .w = w,
        .h = h
    };
    SDL_RenderCopy(ui.renderer, ui.image_buttons, &button_rect, &button_rect);
}

static void draw_knob(SDL_Texture *image)
{
    SDL_Rect knob_rect = {
        .x = KNOB_X,
        .y = KNOB_Y,
        .w = KNOB_WIDTH,
        .h = KNOB_HEIGHT
    };
    SDL_RenderCopy(ui.renderer, image, &knob_rect, &knob_rect);
}

static void draw_printer_line(int x, int y, int line)
{
    SDL_Surface *surface;
    SDL_Texture *texture;
    SDL_Color fg = {0, 0, 0, 255};
    SDL_Color bg = {242, 230, 223, 255};
    SDL_Rect rect = {
        .x = x,
        .y = y + BG_HEIGHT
    };
    char text[PRINTER_LINE_SIZE + 1];
    int len;
    memcpy(text, ui.printer_output[line], PRINTER_LINE_SIZE);
    len = PRINTER_LINE_SIZE;
    while (len > 0 && text[len - 1] == ' ') {
        --len;
    }
    text[len] = '\0';
    if (!len) {
        return;
    }
    surface = TTF_RenderText_Shaded(ui.font, text, fg, bg);
    rect.w = surface->w;
    rect.h = surface->h;
    texture = SDL_CreateTextureFromSurface(ui.renderer, surface);
    SDL_RenderCopy(ui.renderer, texture, NULL, &rect);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

static void draw_cursor(int x, int y)
{
    SDL_Rect rect = {
        .x = x,
        .y = BG_HEIGHT + y + ui.font_height - 2,
        .w = ui.font_width,
        .h = 2
    };
    SDL_SetRenderDrawColor(ui.renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(ui.renderer, &rect);
}

static void draw_screen(void)
{
    SDL_Rect main_rect = {
        .x = 0,
        .y = 0,
        .w = BG_WIDTH,
        .h = BG_HEIGHT
    };
    SDL_Rect printer_rect = {
        .x = 0,
        .y = BG_HEIGHT,
        .w = BG_WIDTH,
        .h = PAPER_HEIGHT
    };
    uint32_t lamps;
    uint32_t selected_register;
    int line;

    /* Get the state of the engine in the background thread */
    SDL_LockMutex(ui.mutex);
    litton_update_status_lights(&machine);
    lamps = machine.status_lights;
    selected_register = machine.selected_register;
    SDL_UnlockMutex(ui.mutex);

    /* Background of the printer region is "paper write" to simulate
     * old printer paper.  Fill the rest with black. */
    SDL_SetRenderDrawColor(ui.renderer, 0, 0, 0, 255);
    SDL_RenderClear(ui.renderer);
    SDL_SetRenderDrawColor(ui.renderer, 242, 230, 223, 255);
    SDL_RenderFillRect(ui.renderer, &printer_rect);

    /* Draw the outline of the controls */
    SDL_RenderCopy(ui.renderer, ui.image_bg, &main_rect, &main_rect);

    /* Draw the lamps that are currently lit */
    draw_lamp(lamps, LITTON_STATUS_POWER, LAMP_POWER_X, LAMP_POWER_Y);
    draw_lamp(lamps, LITTON_STATUS_READY, LAMP_READY_X, LAMP_READY_Y);
    draw_lamp(lamps, LITTON_STATUS_RUN, LAMP_RUN_X, LAMP_RUN_Y);
    draw_lamp(lamps, LITTON_STATUS_HALT, LAMP_HALT_X, LAMP_HALT_Y);
    draw_lamp(lamps, LITTON_STATUS_K, LAMP_K_X, LAMP_K_Y);
    draw_lamp(lamps, LITTON_STATUS_TRACK, LAMP_TRACK_X, LAMP_TRACK_Y);
    draw_lamp(lamps, LITTON_STATUS_BIT_0, LAMP_BIT_0_X, LAMP_BIT_0_Y);
    draw_lamp(lamps, LITTON_STATUS_BIT_1, LAMP_BIT_1_X, LAMP_BIT_1_Y);
    draw_lamp(lamps, LITTON_STATUS_BIT_2, LAMP_BIT_2_X, LAMP_BIT_2_Y);
    draw_lamp(lamps, LITTON_STATUS_BIT_3, LAMP_BIT_3_X, LAMP_BIT_3_Y);
    draw_lamp(lamps, LITTON_STATUS_BIT_4, LAMP_BIT_4_X, LAMP_BIT_4_Y);
    draw_lamp(lamps, LITTON_STATUS_BIT_5, LAMP_BIT_5_X, LAMP_BIT_5_Y);
    draw_lamp(lamps, LITTON_STATUS_BIT_6, LAMP_BIT_6_X, LAMP_BIT_6_Y);
    draw_lamp(lamps, LITTON_STATUS_BIT_7, LAMP_BIT_7_X, LAMP_BIT_7_Y);
    draw_lamp(lamps, LITTON_STATUS_INST, LAMP_INST_X, LAMP_INST_Y);
    draw_lamp(lamps, LITTON_STATUS_ACCUM, LAMP_ACCUM_X, LAMP_ACCUM_Y);

    /* Draw the position of the register select knob */
    switch (selected_register) {
    case LITTON_BUTTON_CONTROL_UP:
        draw_knob(ui.image_control_up);
        break;

    case LITTON_BUTTON_INST_32:
        draw_knob(ui.image_knob_I32);
        break;

    case LITTON_BUTTON_INST_24:
        draw_knob(ui.image_knob_I24);
        break;

    case LITTON_BUTTON_INST_16:
        draw_knob(ui.image_knob_I16);
        break;

    case LITTON_BUTTON_INST_8:
        draw_knob(ui.image_knob_I8);
        break;

    case LITTON_BUTTON_INST_0:
        draw_knob(ui.image_knob_I0);
        break;

    case LITTON_BUTTON_CONTROL_DOWN:
        draw_knob(ui.image_control_down);
        break;

    case LITTON_BUTTON_ACCUM_32:
        draw_knob(ui.image_knob_A32);
        break;

    case LITTON_BUTTON_ACCUM_24:
        draw_knob(ui.image_knob_A24);
        break;

    case LITTON_BUTTON_ACCUM_16:
        draw_knob(ui.image_knob_A16);
        break;

    case LITTON_BUTTON_ACCUM_8:
        draw_knob(ui.image_knob_A8);
        break;

    case LITTON_BUTTON_ACCUM_0:
        draw_knob(ui.image_knob_A0);
        break;
    }

    /* Highlight the push button that is currently pressed */
    switch (ui.pressed_button) {
    case LITTON_BUTTON_POWER:
        draw_pressed_button(BUTTON_POWER_X, BUTTON_POWER_Y);
        break;

    case LITTON_BUTTON_READY:
        draw_pressed_button(BUTTON_READY_X, BUTTON_READY_Y);
        break;

    case LITTON_BUTTON_RUN:
        draw_pressed_button(BUTTON_RUN_X, BUTTON_RUN_Y);
        break;

    case LITTON_BUTTON_HALT:
        draw_pressed_button(BUTTON_HALT_X, BUTTON_HALT_Y);
        break;

    case LITTON_BUTTON_K_RESET:
        draw_pressed_button(BUTTON_K_RESET_X, BUTTON_K_RESET_Y);
        break;

    case LITTON_BUTTON_K_SET:
        draw_pressed_button(BUTTON_K_SET_X, BUTTON_K_SET_Y);
        break;

    case LITTON_BUTTON_RESET:
        draw_pressed_button(BUTTON_BIT_RESET_X, BUTTON_BIT_RESET_Y);
        break;

    case LITTON_BUTTON_BIT_0:
        draw_pressed_button(BUTTON_BIT_0_X, BUTTON_BIT_0_Y);
        break;

    case LITTON_BUTTON_BIT_1:
        draw_pressed_button(BUTTON_BIT_1_X, BUTTON_BIT_1_Y);
        break;

    case LITTON_BUTTON_BIT_2:
        draw_pressed_button(BUTTON_BIT_2_X, BUTTON_BIT_2_Y);
        break;

    case LITTON_BUTTON_BIT_3:
        draw_pressed_button(BUTTON_BIT_3_X, BUTTON_BIT_3_Y);
        break;

    case LITTON_BUTTON_BIT_4:
        draw_pressed_button(BUTTON_BIT_4_X, BUTTON_BIT_4_Y);
        break;

    case LITTON_BUTTON_BIT_5:
        draw_pressed_button(BUTTON_BIT_5_X, BUTTON_BIT_5_Y);
        break;

    case LITTON_BUTTON_BIT_6:
        draw_pressed_button(BUTTON_BIT_6_X, BUTTON_BIT_6_Y);
        break;

    case LITTON_BUTTON_BIT_7:
        draw_pressed_button(BUTTON_BIT_7_X, BUTTON_BIT_7_Y);
        break;

    case LITTON_BUTTON_CONTROL_UP:
        draw_pressed_button_sized
            (BUTTON_CONTROL_UP_X, BUTTON_CONTROL_UP_Y,
             BUTTON_CONTROL_UP_WIDTH, BUTTON_CONTROL_UP_HEIGHT);
        break;

    case LITTON_BUTTON_INST_32:
        draw_pressed_button_sized
            (BUTTON_INST_32_X, BUTTON_INST_32_Y,
             BUTTON_INST_32_WIDTH, BUTTON_INST_32_HEIGHT);
        break;

    case LITTON_BUTTON_INST_24:
        draw_pressed_button_sized
            (BUTTON_INST_24_X, BUTTON_INST_24_Y,
             BUTTON_INST_24_WIDTH, BUTTON_INST_24_HEIGHT);
        break;

    case LITTON_BUTTON_INST_16:
        draw_pressed_button_sized
            (BUTTON_INST_16_X, BUTTON_INST_16_Y,
             BUTTON_INST_16_WIDTH, BUTTON_INST_16_HEIGHT);
        break;

    case LITTON_BUTTON_INST_8:
        draw_pressed_button_sized
            (BUTTON_INST_8_X, BUTTON_INST_8_Y,
             BUTTON_INST_8_WIDTH, BUTTON_INST_8_HEIGHT);
        break;

    case LITTON_BUTTON_INST_0:
        draw_pressed_button_sized
            (BUTTON_INST_0_X, BUTTON_INST_0_Y,
             BUTTON_INST_0_WIDTH, BUTTON_INST_0_HEIGHT);
        break;

    case LITTON_BUTTON_CONTROL_DOWN:
        draw_pressed_button_sized
            (BUTTON_CONTROL_DOWN_X, BUTTON_CONTROL_DOWN_Y,
             BUTTON_CONTROL_DOWN_WIDTH, BUTTON_CONTROL_DOWN_HEIGHT);
        break;

    case LITTON_BUTTON_ACCUM_32:
        draw_pressed_button_sized
            (BUTTON_ACCUM_32_X, BUTTON_ACCUM_32_Y,
             BUTTON_ACCUM_32_WIDTH, BUTTON_ACCUM_32_HEIGHT);
        break;

    case LITTON_BUTTON_ACCUM_24:
        draw_pressed_button_sized
            (BUTTON_ACCUM_24_X, BUTTON_ACCUM_24_Y,
             BUTTON_ACCUM_24_WIDTH, BUTTON_ACCUM_24_HEIGHT);
        break;

    case LITTON_BUTTON_ACCUM_16:
        draw_pressed_button_sized
            (BUTTON_ACCUM_16_X, BUTTON_ACCUM_16_Y,
             BUTTON_ACCUM_16_WIDTH, BUTTON_ACCUM_16_HEIGHT);
        break;

    case LITTON_BUTTON_ACCUM_8:
        draw_pressed_button_sized
            (BUTTON_ACCUM_8_X, BUTTON_ACCUM_8_Y,
             BUTTON_ACCUM_8_WIDTH, BUTTON_ACCUM_8_HEIGHT);
        break;

    case LITTON_BUTTON_ACCUM_0:
        draw_pressed_button_sized
            (BUTTON_ACCUM_0_X, BUTTON_ACCUM_0_Y,
             BUTTON_ACCUM_0_WIDTH, BUTTON_ACCUM_0_HEIGHT);
        break;

    case LITTON_BUTTON_DRUM_LOAD:
        draw_pressed_button(BUTTON_DRUM_LOAD_X, BUTTON_DRUM_LOAD_Y);
        break;

    case LITTON_BUTTON_DRUM_SAVE:
        draw_pressed_button(BUTTON_DRUM_SAVE_X, BUTTON_DRUM_SAVE_Y);
        break;

    case LITTON_BUTTON_TAPE_IN:
        draw_pressed_button(BUTTON_TAPE_IN_X, BUTTON_TAPE_IN_Y);
        break;

    case LITTON_BUTTON_TAPE_OUT:
        draw_pressed_button(BUTTON_TAPE_OUT_X, BUTTON_TAPE_OUT_Y);
        break;
    }

    /* Draw the text for the printer output */
    for (line = 0; line < PRINTER_MAX_LINES; ++line) {
        draw_printer_line(5, 5 + line * ui.font_height, line);
    }

    /* Draw the cursor at the current print position */
    draw_cursor(5 + ui.printer_column * ui.font_width,
                5 + ui.printer_line * ui.font_height);

    /* Flip the screen and display what we just drew */
    SDL_RenderPresent(ui.renderer);
}

static int in_button_rect
    (int x, int y, int rectx, int recty, int rectw, int recth)
{
    return x >= rectx && x < (rectx + rectw) &&
           y >= recty && y < (recty + recth);
}

static uint32_t get_button(int x, int y)
{
    if (in_button_rect
            (x, y, BUTTON_POWER_X, BUTTON_POWER_Y,
             BUTTON_WIDTH, BUTTON_HEIGHT)) {
        return LITTON_BUTTON_POWER;
    } else if (in_button_rect
            (x, y, BUTTON_READY_X, BUTTON_READY_Y,
             BUTTON_WIDTH, BUTTON_HEIGHT)) {
        return LITTON_BUTTON_READY;
    } else if (in_button_rect
            (x, y, BUTTON_RUN_X, BUTTON_RUN_Y,
             BUTTON_WIDTH, BUTTON_HEIGHT)) {
        return LITTON_BUTTON_RUN;
    } else if (in_button_rect
            (x, y, BUTTON_HALT_X, BUTTON_HALT_Y,
             BUTTON_WIDTH, BUTTON_HEIGHT)) {
        return LITTON_BUTTON_HALT;
    } else if (in_button_rect
            (x, y, BUTTON_K_RESET_X, BUTTON_K_RESET_Y,
             BUTTON_WIDTH, BUTTON_HEIGHT)) {
        return LITTON_BUTTON_K_RESET;
    } else if (in_button_rect
            (x, y, BUTTON_K_SET_X, BUTTON_K_SET_Y,
             BUTTON_WIDTH, BUTTON_HEIGHT)) {
        return LITTON_BUTTON_K_SET;
    } else if (in_button_rect
            (x, y, BUTTON_BIT_RESET_X, BUTTON_BIT_RESET_Y,
             BUTTON_WIDTH, BUTTON_HEIGHT)) {
        return LITTON_BUTTON_RESET;
    } else if (in_button_rect
            (x, y, BUTTON_BIT_0_X, BUTTON_BIT_0_Y,
             BUTTON_WIDTH, BUTTON_HEIGHT)) {
        return LITTON_BUTTON_BIT_0;
    } else if (in_button_rect
            (x, y, BUTTON_BIT_1_X, BUTTON_BIT_1_Y,
             BUTTON_WIDTH, BUTTON_HEIGHT)) {
        return LITTON_BUTTON_BIT_1;
    } else if (in_button_rect
            (x, y, BUTTON_BIT_2_X, BUTTON_BIT_2_Y,
             BUTTON_WIDTH, BUTTON_HEIGHT)) {
        return LITTON_BUTTON_BIT_2;
    } else if (in_button_rect
            (x, y, BUTTON_BIT_3_X, BUTTON_BIT_3_Y,
             BUTTON_WIDTH, BUTTON_HEIGHT)) {
        return LITTON_BUTTON_BIT_3;
    } else if (in_button_rect
            (x, y, BUTTON_BIT_4_X, BUTTON_BIT_4_Y,
             BUTTON_WIDTH, BUTTON_HEIGHT)) {
        return LITTON_BUTTON_BIT_4;
    } else if (in_button_rect
            (x, y, BUTTON_BIT_5_X, BUTTON_BIT_5_Y,
             BUTTON_WIDTH, BUTTON_HEIGHT)) {
        return LITTON_BUTTON_BIT_5;
    } else if (in_button_rect
            (x, y, BUTTON_BIT_6_X, BUTTON_BIT_6_Y,
             BUTTON_WIDTH, BUTTON_HEIGHT)) {
        return LITTON_BUTTON_BIT_6;
    } else if (in_button_rect
            (x, y, BUTTON_BIT_7_X, BUTTON_BIT_7_Y,
             BUTTON_WIDTH, BUTTON_HEIGHT)) {
        return LITTON_BUTTON_BIT_7;
    } else if (in_button_rect
            (x, y, BUTTON_CONTROL_UP_X, BUTTON_CONTROL_UP_Y,
             BUTTON_CONTROL_UP_WIDTH, BUTTON_CONTROL_UP_HEIGHT)) {
        return LITTON_BUTTON_CONTROL_UP;
    } else if (in_button_rect
            (x, y, BUTTON_CONTROL_DOWN_X, BUTTON_CONTROL_DOWN_Y,
             BUTTON_CONTROL_DOWN_WIDTH, BUTTON_CONTROL_DOWN_HEIGHT)) {
        return LITTON_BUTTON_CONTROL_DOWN;
    } else if (in_button_rect
            (x, y, BUTTON_INST_32_X, BUTTON_INST_32_Y,
             BUTTON_INST_32_WIDTH, BUTTON_INST_32_HEIGHT)) {
        return LITTON_BUTTON_INST_32;
    } else if (in_button_rect
            (x, y, BUTTON_INST_24_X, BUTTON_INST_24_Y,
             BUTTON_INST_24_WIDTH, BUTTON_INST_24_HEIGHT)) {
        return LITTON_BUTTON_INST_24;
    } else if (in_button_rect
            (x, y, BUTTON_INST_16_X, BUTTON_INST_16_Y,
             BUTTON_INST_16_WIDTH, BUTTON_INST_16_HEIGHT)) {
        return LITTON_BUTTON_INST_16;
    } else if (in_button_rect
            (x, y, BUTTON_INST_8_X, BUTTON_INST_8_Y,
             BUTTON_INST_8_WIDTH, BUTTON_INST_8_HEIGHT)) {
        return LITTON_BUTTON_INST_8;
    } else if (in_button_rect
            (x, y, BUTTON_INST_0_X, BUTTON_INST_0_Y,
             BUTTON_INST_0_WIDTH, BUTTON_INST_0_HEIGHT)) {
        return LITTON_BUTTON_INST_0;
    } else if (in_button_rect
            (x, y, BUTTON_ACCUM_32_X, BUTTON_ACCUM_32_Y,
             BUTTON_ACCUM_32_WIDTH, BUTTON_ACCUM_32_HEIGHT)) {
        return LITTON_BUTTON_ACCUM_32;
    } else if (in_button_rect
            (x, y, BUTTON_ACCUM_24_X, BUTTON_ACCUM_24_Y,
             BUTTON_ACCUM_24_WIDTH, BUTTON_ACCUM_24_HEIGHT)) {
        return LITTON_BUTTON_ACCUM_24;
    } else if (in_button_rect
            (x, y, BUTTON_ACCUM_16_X, BUTTON_ACCUM_16_Y,
             BUTTON_ACCUM_16_WIDTH, BUTTON_ACCUM_16_HEIGHT)) {
        return LITTON_BUTTON_ACCUM_16;
    } else if (in_button_rect
            (x, y, BUTTON_ACCUM_8_X, BUTTON_ACCUM_8_Y,
             BUTTON_ACCUM_8_WIDTH, BUTTON_ACCUM_8_HEIGHT)) {
        return LITTON_BUTTON_ACCUM_8;
    } else if (in_button_rect
            (x, y, BUTTON_ACCUM_0_X, BUTTON_ACCUM_0_Y,
             BUTTON_ACCUM_0_WIDTH, BUTTON_ACCUM_0_HEIGHT)) {
        return LITTON_BUTTON_ACCUM_0;
    } else if (in_button_rect
            (x, y, BUTTON_DRUM_LOAD_X, BUTTON_DRUM_LOAD_Y,
             BUTTON_WIDTH, BUTTON_HEIGHT)) {
        return LITTON_BUTTON_DRUM_LOAD;
    } else if (in_button_rect
            (x, y, BUTTON_DRUM_SAVE_X, BUTTON_DRUM_SAVE_Y,
             BUTTON_WIDTH, BUTTON_HEIGHT)) {
        return LITTON_BUTTON_DRUM_SAVE;
    } else if (in_button_rect
            (x, y, BUTTON_TAPE_IN_X, BUTTON_TAPE_IN_Y,
             BUTTON_WIDTH, BUTTON_HEIGHT)) {
        return LITTON_BUTTON_TAPE_IN;
    } else if (in_button_rect
            (x, y, BUTTON_TAPE_OUT_X, BUTTON_TAPE_OUT_Y,
             BUTTON_WIDTH, BUTTON_HEIGHT)) {
        return LITTON_BUTTON_TAPE_OUT;
    }
    return 0;
}

static void print_line_feed()
{
    ++(ui.printer_line);
    if (ui.printer_line >= PRINTER_MAX_LINES) {
        memmove(ui.printer_output[0], ui.printer_output[1],
                (PRINTER_LINE_SIZE + 1) * (PRINTER_MAX_LINES - 1));
        memset(ui.printer_output[PRINTER_MAX_LINES - 1], ' ',
               PRINTER_LINE_SIZE);
        --(ui.printer_line);
    }
}

static void print_ascii(uint8_t ch)
{
    /* Uncomment to print to stdout at the same time as the window.
    putc(ch, stdout);
    fflush(stdout);
    */
    if (ch == '\r') {
        ui.printer_column = 0;
    } else if (ch == '\n') {
        print_line_feed();
    } else if (ch == '\b') {
        if (ui.printer_column > 0) {
            --(ui.printer_column);
        }
    } else {
        if (ui.printer_column >= PRINTER_LINE_SIZE) {
            ui.printer_column = 0;
            print_line_feed();
        }
        ui.printer_output[ui.printer_line][ui.printer_column] = ch;
        ++(ui.printer_column);
    }
}

static void print_string(const char *str)
{
    while (*str != '\0') {
        print_ascii(*str);
        ++str;
    }
}

static void printer_output
    (litton_state_t *state, litton_device_t *device,
     uint8_t value, litton_parity_t parity)
{
    (void)device;
    if (state->printer_charset != LITTON_CHARSET_HEX) {
        value = litton_remove_parity(value, parity);
    }
    if (state->printer_charset == LITTON_CHARSET_EBS1231) {
        /* Does this look like a print wheel position? */
        uint8_t position = litton_print_wheel_position(value);
        if (position != 0) {
            /* Move the print head on the current line */
            --position;
            if (position >= PRINTER_LINE_SIZE) {
                position = PRINTER_LINE_SIZE - 1;
            }
            ui.printer_column = position;
        } else if (value == 075 || value == 055 || value == 054) {
            /* Line Feed Left / Line Feed Right / Line Feed Both */
            print_ascii('\n');
        } else if (value == 056 || value == 074) {
            /* Change ribbon color - ignored for now */
        } else {
            /* Convert the code into its ASCII form */
            const char *string_form;
            int ch = litton_char_from_charset
                (value, device->charset, &string_form);
            if (ch == '\f') {
                /* Form feed; just output a carriage return and line feed */
                print_ascii('\r');
                print_ascii('\n');
            } else if (ch >= 0) {
                /* Single character */
                print_ascii(ch);
            } else if (ch == -2) {
                /* Multi-character string */
                print_string(string_form);
            }
        }
    } else if (state->printer_charset == LITTON_CHARSET_HEX) {
        /* Output bytes in hexadecimal */
        static const char hex_chars[] = "0123456789ABCDEF";
        if (ui.printer_column > 0) {
            print_ascii(' ');
        }
        print_ascii(hex_chars[(value >> 4) & 0x0F]);
        print_ascii(hex_chars[value & 0x0F]);
        if (ui.printer_column >= 47) {
            print_ascii('\r');
            print_ascii('\n');
        }
    } else {
        /* Assume plain ASCII codes as input */
        print_ascii(value);
    }
}

static void process_input_char(uint8_t value)
{
    if (ui.keyboard_count < KEYBOARD_BUFFER_SIZE) {
        ui.keyboard_input[(ui.keyboard_count)++] = value;
    } else {
        /* Keyboard input buffer is full, so drop the oldest character */
        memmove(ui.keyboard_input, ui.keyboard_input + 1,
                KEYBOARD_BUFFER_SIZE - 1);
        ui.keyboard_input[KEYBOARD_BUFFER_SIZE - 1] = value;
    }
}

static void process_ascii_input(char ch, int allow_control_chars)
{
    size_t posn = 0;
    int ch2;
    if ((ch & 0xFF) < 0x20 && !allow_control_chars) {
        return;
    }
    ch2 = litton_char_to_charset(&ch, &posn, 1, machine.keyboard_charset);
    if (ch2 >= 0) {
        process_input_char((uint8_t)ch2);
    }
}

static void process_ebs1231_code(uint8_t value)
{
    if (machine.keyboard_charset == LITTON_CHARSET_EBS1231) {
        process_input_char(value);
    }
}

static void process_text_input(const char *text)
{
    if (!litton_is_halted(&machine)) {
        while (*text != '\0') {
            process_ascii_input(*text, 0);
            ++text;
        }
    }
}

static void process_key(SDL_Keysym keysym)
{
    if (litton_is_halted(&machine)) {
        /* Keyboard input is suppressed when the machine is halted */
        return;
    }
    switch (keysym.sym) {
    case SDLK_RETURN:
    case SDLK_RETURN2:
    case SDLK_KP_ENTER:
        process_ascii_input('\r', 1);
        break;

    case SDLK_BACKSPACE:
    case SDLK_KP_BACKSPACE:
        process_ascii_input('\b', 1);
        break;

    case SDLK_h:
        /* CTRL-H - backspace */
        if ((keysym.mod & KMOD_CTRL) != 0) {
            process_ascii_input('\b', 1);
        }
        break;

    case SDLK_j:
        /* CTRL-J - line feed */
        if ((keysym.mod & KMOD_CTRL) != 0) {
            process_ascii_input('\n', 1);
        }
        break;

    case SDLK_l:
        /* CTRL-L - form feed */
        if ((keysym.mod & KMOD_CTRL) != 0) {
            process_ascii_input('\f', 1);
        }
        break;

    case SDLK_m:
        /* CTRL-M - carriage return */
        if ((keysym.mod & KMOD_CTRL) != 0) {
            process_ascii_input('\r', 1);
        }
        break;

    case SDLK_F1:
        if ((keysym.mod & KMOD_SHIFT) != 0) {
            process_ebs1231_code(0134); /* SHIFT-I */
        } else {
            process_ebs1231_code(034);  /* I */
        }
        break;

    case SDLK_F2:
        if ((keysym.mod & KMOD_SHIFT) != 0) {
            process_ebs1231_code(0135); /* SHIFT-II */
        } else {
            process_ebs1231_code(035);  /* II */
        }
        break;

    case SDLK_F3:
        if ((keysym.mod & KMOD_SHIFT) != 0) {
            process_ebs1231_code(0136); /* SHIFT-III */
        } else {
            process_ebs1231_code(036);  /* III */
        }
        break;

    case SDLK_F4:
        if ((keysym.mod & KMOD_SHIFT) != 0) {
            process_ebs1231_code(0137); /* SHIFT-IIII */
        } else {
            process_ebs1231_code(037);  /* IIII */
        }
        break;

    case SDLK_F5:
        if ((keysym.mod & KMOD_SHIFT) != 0) {
            process_ebs1231_code(0114); /* SHIFT-P1 */
        } else {
            process_ebs1231_code(014);  /* P1 */
        }
        break;

    case SDLK_F6:
        if ((keysym.mod & KMOD_SHIFT) != 0) {
            process_ebs1231_code(0115); /* SHIFT-P2 */
        } else {
            process_ebs1231_code(015);  /* P2 */
        }
        break;

    case SDLK_F7:
        if ((keysym.mod & KMOD_SHIFT) != 0) {
            process_ebs1231_code(0116); /* SHIFT-P3 */
        } else {
            process_ebs1231_code(016);  /* P3 */
        }
        break;

    case SDLK_F8:
        if ((keysym.mod & KMOD_SHIFT) != 0) {
            process_ebs1231_code(0117); /* SHIFT-P4 */
        } else {
            process_ebs1231_code(017);  /* P4 */
        }
        break;

    case SDLK_UP:
        if ((keysym.mod & KMOD_CTRL) != 0) {
            process_ebs1231_code(055); /* Line Feed Right */
        } else {
            process_ebs1231_code(075); /* Line Feed Left */
        }
        break;

    case SDLK_PAGEUP:
        process_ebs1231_code(054); /* Line Feed Both */
        break;
    }
}

static int keyboard_input
    (litton_state_t *state, litton_device_t *device,
     uint8_t *value, litton_parity_t parity)
{
    (void)state;
    (void)device;
    if (ui.keyboard_count > 0) {
        *value = litton_add_parity(ui.keyboard_input[0], parity);
        --(ui.keyboard_count);
        memmove(ui.keyboard_input, ui.keyboard_input + 1, ui.keyboard_count);
        return 1;
    }
    return 0;
}

static void create_devices(void)
{
    litton_device_t *device;

    /* Create the printer device for redirecting output to the UI */
    device = calloc(1, sizeof(litton_device_t));
    device->id = LITTON_DEVICE_PRINTER;
    device->supports_input = 0;
    device->supports_output = 1;
    device->charset = machine.printer_charset;
    device->output = printer_output;
    litton_add_device(&machine, device);
    memset(ui.printer_output, ' ', sizeof(ui.printer_output));

    /* Start at the bottom of the print area and gradually scroll up */
    ui.printer_column = 0;
    ui.printer_line = PRINTER_MAX_LINES - 1;

    /* Create the keyboard device for redirecting input from the UI */
    ui.keyboard_count = 0;
    device = calloc(1, sizeof(litton_device_t));
    device->id = LITTON_DEVICE_KEYBOARD;
    device->supports_input = 1;
    device->supports_output = 0;
    device->charset = machine.keyboard_charset;
    device->input = keyboard_input;
    litton_add_device(&machine, device);
}

/* Use the external tool "zenity" to handle the file dialog */
#define DRUM_LOAD_CMD "zenity --file-selection --file-filter='*.drum'"
#define DRUM_SAVE_CMD "zenity --file-selection --save --confirm-overwrite --file-filter='*.drum'"
#define TAPE_IN_CMD "zenity --file-selection --file-filter='*.tape'"
#define TAPE_OUT_CMD "zenity --file-selection --save --confirm-overwrite --file-filter='*.tape'"

static char *ask_for_filename(const char *cmdline)
{
    FILE *file = popen(cmdline, "r");
    char buffer[BUFSIZ];
    size_t len;
    int error;
    if (!file) {
        print_string("popen error");
        return 0;
    }
    error = (fgets(buffer, sizeof(buffer), file) == NULL);
    error |= (pclose(file) != 0);
    if (error) {
        return 0;
    }
    len = strlen(buffer);
    while (len > 0 && (buffer[len - 1] == '\r' || buffer[len - 1] == '\n')) {
        --len;
    }
    buffer[len] = '\0';
    return strdup(buffer);
}

static void handle_other_button(uint32_t button)
{
    char *filename = 0;
    if (!litton_is_halted(&machine)) {
        /* Machine must be halted for this */
        return;
    }
    switch (button) {
    case LITTON_BUTTON_DRUM_LOAD:
        filename = ask_for_filename(DRUM_LOAD_CMD);
        if (filename) {
            SDL_LockMutex(ui.mutex);
            litton_clear_memory(&machine);
            if (litton_load_drum(&machine, filename, 0)) {
                litton_reset(&machine);
                SDL_UnlockMutex(ui.mutex);
                print_string(filename);
                print_string(" loaded\r\n");
            } else {
                SDL_UnlockMutex(ui.mutex);
                print_string(filename);
                print_string(" failed to load\r\n");
            }
        }
        break;

    case LITTON_BUTTON_DRUM_SAVE:
        filename = ask_for_filename(DRUM_SAVE_CMD);
        if (filename) {
            SDL_LockMutex(ui.mutex);
            if (litton_save_drum(&machine, filename)) {
                SDL_UnlockMutex(ui.mutex);
                print_string(filename);
                print_string(" saved\r\n");
            } else {
                SDL_UnlockMutex(ui.mutex);
                print_string(filename);
                print_string(" failed to save\r\n");
            }
        }
        break;

    case LITTON_BUTTON_TAPE_IN:
        filename = ask_for_filename(TAPE_IN_CMD);
        // TODO
        break;

    case LITTON_BUTTON_TAPE_OUT:
        filename = ask_for_filename(TAPE_OUT_CMD);
        // TODO
        break;
    }
    if (filename) {
        free(filename);
    }
}

static int run_litton(void *data)
{
    litton_state_t *state = (litton_state_t *)data;
    int was_running = 0;
    uint64_t elapsed_ns;
    uint64_t checkpoint_counter;
    struct timespec checkpoint_time;
    struct timespec sleep_to_time;
    struct timespec now_time;

    checkpoint_counter = state->cycle_counter;
    clock_gettime(CLOCK_MONOTONIC, &checkpoint_time);

    while (!ui.quit) {
        SDL_LockMutex(ui.mutex);
        if (litton_is_halted(state)) {
            /* Nothing to do if we are not currently running */
            SDL_UnlockMutex(ui.mutex);
            usleep(20000); /* Sleep for 20ms and check again */
            was_running = 0;

            /* Keyboard input is suppressed when halted */
            ui.keyboard_count = 0;
        } else {
            /* Re-establish the checkpoint if we just started running */
            if (!was_running) {
                checkpoint_counter = state->cycle_counter;
                clock_gettime(CLOCK_MONOTONIC, &checkpoint_time);
                was_running = 1;
            }

            /* Run a single instruction step */
            litton_step(state);
            litton_update_status_lights(state);
            SDL_UnlockMutex(ui.mutex);

            /* Simulate the actual speed of the computer */
            elapsed_ns = (state->cycle_counter - checkpoint_counter) * 1000;
            sleep_to_time = checkpoint_time;
            sleep_to_time.tv_nsec += elapsed_ns % 1000000000;
            sleep_to_time.tv_sec += elapsed_ns / 1000000000;
            while (sleep_to_time.tv_nsec >= 1000000000) {
                sleep_to_time.tv_nsec -= 1000000000;
                ++(sleep_to_time.tv_sec);
            }
            clock_gettime(CLOCK_MONOTONIC, &now_time);
            if (state->acceleration_counter != 0 ||
                    now_time.tv_sec > sleep_to_time.tv_sec ||
                    (now_time.tv_sec == sleep_to_time.tv_sec &&
                     now_time.tv_nsec >= sleep_to_time.tv_nsec)) {
                /* Deadline has already passed, so resynchronise on "now" */
                checkpoint_counter = state->cycle_counter;
                checkpoint_time = now_time;
            } else {
                clock_nanosleep
                    (CLOCK_MONOTONIC, TIMER_ABSTIME, &sleep_to_time, NULL);
            }
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
    const char *progname = argv[0];
    const char *drum_image;
    int maximized_mode = 0;
    int exit_status = 0;
    int width, height;
    int wait_status;
    int opt;
    SDL_Event event;
    SDL_Color color = {0, 0, 0, 255};
    SDL_Surface *surface;

    /* Initialize the machine */
    litton_init(&machine);

    /* Process the command-line options */
    while ((opt = getopt(argc, argv, "mv")) != -1) {
        if (opt == 'm') {
            maximized_mode = 1;
        } else if (opt == 'v') {
            machine.disassemble = 1;
        } else {
            usage(progname);
            litton_free(&machine);
            return 1;
        }
    }

    /* Load the drum image into memory */
    if (optind < argc) {
        drum_image = argv[optind];
        if (!litton_load_drum(&machine, drum_image, NULL)) {
            litton_free(&machine);
            return 1;
        }
    }
    create_devices();

    /* Create the SDL infrastructure for video output */
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Could not initialise SDL: %s\n", SDL_GetError());
        return 1;
    }
    width = BG_WIDTH;
    height = BG_HEIGHT + PAPER_HEIGHT;
    ui.window = SDL_CreateWindow(
        "Litton Emulator",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        width, height,
        SDL_WINDOW_RESIZABLE |
        (maximized_mode ? SDL_WINDOW_MAXIMIZED : 0)
    );
    if (!ui.window) {
        fprintf(stderr, "Failed to create SDL window: %s\n", SDL_GetError());
        litton_free(&machine);
        return 1;
    }
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");
    ui.renderer = SDL_CreateRenderer
        (ui.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!ui.renderer) {
        fprintf(stderr, "Failed to create SDL renderer: %s\n", SDL_GetError());
        litton_free(&machine);
        return 1;
    }
    SDL_RenderSetLogicalSize(ui.renderer, width, height);

    /* Load the images */
    ui.image_bg = IMG_LoadTexture_RW
        (ui.renderer,
         SDL_RWFromConstMem(front_panel_background_png,
                            front_panel_background_png_len), 1);
    ui.image_lamps = IMG_LoadTexture_RW
        (ui.renderer,
         SDL_RWFromConstMem(front_panel_lamps_lit_png,
                            front_panel_lamps_lit_png_len), 1);
    ui.image_buttons = IMG_LoadTexture_RW
        (ui.renderer,
         SDL_RWFromConstMem(front_panel_buttons_pressed_png,
                            front_panel_buttons_pressed_png_len), 1);
    ui.image_control_up = IMG_LoadTexture_RW
        (ui.renderer,
         SDL_RWFromConstMem(front_panel_knob_control_up_png,
                            front_panel_knob_control_up_png_len), 1);
    ui.image_control_down = IMG_LoadTexture_RW
        (ui.renderer,
         SDL_RWFromConstMem(front_panel_knob_control_down_png,
                            front_panel_knob_control_down_png_len), 1);
    ui.image_knob_A0 = IMG_LoadTexture_RW
        (ui.renderer,
         SDL_RWFromConstMem(front_panel_knob_A_0_png,
                            front_panel_knob_A_0_png_len), 1);
    ui.image_knob_A8 = IMG_LoadTexture_RW
        (ui.renderer,
         SDL_RWFromConstMem(front_panel_knob_A_8_png,
                            front_panel_knob_A_8_png_len), 1);
    ui.image_knob_A16 = IMG_LoadTexture_RW
        (ui.renderer,
         SDL_RWFromConstMem(front_panel_knob_A_16_png,
                            front_panel_knob_A_16_png_len), 1);
    ui.image_knob_A24 = IMG_LoadTexture_RW
        (ui.renderer,
         SDL_RWFromConstMem(front_panel_knob_A_24_png,
                            front_panel_knob_A_24_png_len), 1);
    ui.image_knob_A32 = IMG_LoadTexture_RW
        (ui.renderer,
         SDL_RWFromConstMem(front_panel_knob_A_32_png,
                            front_panel_knob_A_32_png_len), 1);
    ui.image_knob_I0 = IMG_LoadTexture_RW
        (ui.renderer,
         SDL_RWFromConstMem(front_panel_knob_I_0_png,
                            front_panel_knob_I_0_png_len), 1);
    ui.image_knob_I8 = IMG_LoadTexture_RW
        (ui.renderer,
         SDL_RWFromConstMem(front_panel_knob_I_8_png,
                            front_panel_knob_I_8_png_len), 1);
    ui.image_knob_I16 = IMG_LoadTexture_RW
        (ui.renderer,
         SDL_RWFromConstMem(front_panel_knob_I_16_png,
                            front_panel_knob_I_16_png_len), 1);
    ui.image_knob_I24 = IMG_LoadTexture_RW
        (ui.renderer,
         SDL_RWFromConstMem(front_panel_knob_I_24_png,
                            front_panel_knob_I_24_png_len), 1);
    ui.image_knob_I32 = IMG_LoadTexture_RW
        (ui.renderer,
         SDL_RWFromConstMem(front_panel_knob_I_32_png,
                            front_panel_knob_I_32_png_len), 1);

    /* Need text input to get ASCII out of the keypresses */
    SDL_StartTextInput();

    /* Load the font for the printer output */
    TTF_Init();
    ui.font = TTF_OpenFontRW
        (SDL_RWFromConstMem(___fonts_DotMatrix_Regular_ttf,
                            ___fonts_DotMatrix_Regular_ttf_len), 1, 20);
    surface = TTF_RenderText_Solid(ui.font, "LITTON", color);
    ui.font_width = surface->w / 6;
    ui.font_height = surface->h;
    SDL_FreeSurface(surface);

    /* Create the background thread for running Litton programs */
    ui.mutex = SDL_CreateMutex();

    /* Reset the machine */
    litton_reset(&machine);

    /* Create the run thread */
    ui.run_thread = SDL_CreateThread(run_litton, "litton", &machine);

    /* Main SDL loop */
    ui.quit = 0;
    while (!ui.quit) {
        /* Draw the current screen contents */
        draw_screen();

        /* Process input events */
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                ui.quit = 1;
            } else if (event.type == SDL_MOUSEBUTTONDOWN &&
                       ui.selected_button == 0) {
                ui.selected_button = get_button(event.button.x, event.button.y);
                ui.pressed_button = ui.selected_button;
            } else if (event.type == SDL_MOUSEBUTTONUP &&
                       ui.selected_button != 0) {
                if (ui.pressed_button == ui.selected_button) {
                    SDL_LockMutex(ui.mutex);
                    litton_press_button(&machine, ui.selected_button);
                    SDL_UnlockMutex(ui.mutex);
                    handle_other_button(ui.selected_button);
                }
                ui.pressed_button = 0;
                ui.selected_button = 0;
            } else if (event.type == SDL_MOUSEMOTION &&
                       ui.selected_button != 0) {
                ui.pressed_button = get_button(event.button.x, event.button.y);
                if (ui.pressed_button != ui.selected_button) {
                    ui.pressed_button = 0;
                }
            } else if (event.type == SDL_TEXTINPUT) {
                process_text_input(event.text.text);
            } else if (event.type == SDL_KEYDOWN) {
                process_key(event.key.keysym);
            }
        }
    }

    /* Wait for the background thread to stop */
    SDL_WaitThread(ui.run_thread, &wait_status);

    /* Clean up and exit */
    SDL_DestroyTexture(ui.image_bg);
    SDL_DestroyTexture(ui.image_lamps);
    SDL_DestroyTexture(ui.image_buttons);
    SDL_DestroyTexture(ui.image_control_up);
    SDL_DestroyTexture(ui.image_control_down);
    SDL_DestroyTexture(ui.image_knob_A0);
    SDL_DestroyTexture(ui.image_knob_A8);
    SDL_DestroyTexture(ui.image_knob_A16);
    SDL_DestroyTexture(ui.image_knob_A24);
    SDL_DestroyTexture(ui.image_knob_A32);
    SDL_DestroyTexture(ui.image_knob_I0);
    SDL_DestroyTexture(ui.image_knob_I8);
    SDL_DestroyTexture(ui.image_knob_I16);
    SDL_DestroyTexture(ui.image_knob_I24);
    SDL_DestroyTexture(ui.image_knob_I32);
    TTF_CloseFont(ui.font);
    SDL_DestroyRenderer(ui.renderer);
    SDL_DestroyWindow(ui.window);
    SDL_DestroyMutex(ui.mutex);
    TTF_Quit();
    litton_free(&machine);
    return exit_status;
}
