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

#ifndef LITTON_SDL_IMAGES_H
#define LITTON_SDL_IMAGES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Width of the background image and the entire front panel */
#define BG_WIDTH 1555

/* Height of the background image and the entire front panel */
#define BG_HEIGHT 672

/* Height of the paper area for printer output */
#define PAPER_HEIGHT 250

/* Size of the status lamps */
#define LAMP_WIDTH 32
#define LAMP_HEIGHT 32

/* Size of the buttons */
#define BUTTON_WIDTH 54
#define BUTTON_HEIGHT 54

/* Position of the POWER lamp */
#define LAMP_POWER_X 1391
#define LAMP_POWER_Y 184

/* Position of the POWER button */
#define BUTTON_POWER_X 1379
#define BUTTON_POWER_Y 268

/* Position of the READY lamp */
#define LAMP_READY_X 1337
#define LAMP_READY_Y 184

/* Position of the READY button */
#define BUTTON_READY_X 1326
#define BUTTON_READY_Y 268

/* Position of the RUN lamp */
#define LAMP_RUN_X 1228
#define LAMP_RUN_Y 184

/* Position of the RUN button */
#define BUTTON_RUN_X 1217
#define BUTTON_RUN_Y 268

/* Position of the HALT lamp */
#define LAMP_HALT_X 1173
#define LAMP_HALT_Y 184

/* Position of the HALT button */
#define BUTTON_HALT_X 1161
#define BUTTON_HALT_Y 268

/* Position of the K lamp */
#define LAMP_K_X 1037
#define LAMP_K_Y 184

/* Position of the K RESET button */
#define BUTTON_K_RESET_X 1053
#define BUTTON_K_RESET_Y 268

/* Position of the K SET button */
#define BUTTON_K_SET_X 997
#define BUTTON_K_SET_Y 268

/* Position of the TRACK lamp */
#define LAMP_TRACK_X 956
#define LAMP_TRACK_Y 184

/* Position of the BIT 0 lamp */
#define LAMP_BIT_0_X 901
#define LAMP_BIT_0_Y 184

/* Position of the BIT 0 button */
#define BUTTON_BIT_0_X 889
#define BUTTON_BIT_0_Y 268

/* Position of the BIT 1 lamp */
#define LAMP_BIT_1_X 847
#define LAMP_BIT_1_Y 184

/* Position of the BIT 1 button */
#define BUTTON_BIT_1_X 836
#define BUTTON_BIT_1_Y 268

/* Position of the BIT 2 lamp */
#define LAMP_BIT_2_X 792
#define LAMP_BIT_2_Y 184

/* Position of the BIT 2 button */
#define BUTTON_BIT_2_X 781
#define BUTTON_BIT_2_Y 268

/* Position of the BIT 3 lamp */
#define LAMP_BIT_3_X 738
#define LAMP_BIT_3_Y 184

/* Position of the BIT 3 button */
#define BUTTON_BIT_3_X 726
#define BUTTON_BIT_3_Y 268

/* Position of the BIT 4 lamp */
#define LAMP_BIT_4_X 684
#define LAMP_BIT_4_Y 184

/* Position of the BIT 4 button */
#define BUTTON_BIT_4_X 671
#define BUTTON_BIT_4_Y 268

/* Position of the BIT 5 lamp */
#define LAMP_BIT_5_X 630
#define LAMP_BIT_5_Y 184

/* Position of the BIT 5 button */
#define BUTTON_BIT_5_X 616
#define BUTTON_BIT_5_Y 268

/* Position of the BIT 6 lamp */
#define LAMP_BIT_6_X 574
#define LAMP_BIT_6_Y 184

/* Position of the BIT 6 button */
#define BUTTON_BIT_6_X 563
#define BUTTON_BIT_6_Y 268

/* Position of the BIT 7 lamp */
#define LAMP_BIT_7_X 519
#define LAMP_BIT_7_Y 184

/* Position of the BIT 6 button */
#define BUTTON_BIT_7_X 510
#define BUTTON_BIT_7_Y 268

/* Position of the BIT RESET button */
#define BUTTON_BIT_RESET_X 454
#define BUTTON_BIT_RESET_Y 268

/* Position of the DRUM LOAD button */
#define BUTTON_DRUM_LOAD_X 454
#define BUTTON_DRUM_LOAD_Y 495

/* Position of the DRUM SAVE button */
#define BUTTON_DRUM_SAVE_X 510
#define BUTTON_DRUM_SAVE_Y 495

/* Position of the TAPE IN button */
#define BUTTON_TAPE_IN_X 616
#define BUTTON_TAPE_IN_Y 495

/* Position of the TAPE OUT button */
#define BUTTON_TAPE_OUT_X 671
#define BUTTON_TAPE_OUT_Y 495

/* Position of the INSTRUCTION lamp */
#define LAMP_INST_X 311
#define LAMP_INST_Y 122

/* Position of the ACCUMULATOR lamp */
#define LAMP_ACCUM_X 163
#define LAMP_ACCUM_Y 122

/* Bounding box for the control knob */
#define KNOB_X 168
#define KNOB_Y 229
#define KNOB_WIDTH 160
#define KNOB_HEIGHT 160

/* Position of the control up button */
#define BUTTON_CONTROL_UP_X 207
#define BUTTON_CONTROL_UP_Y 150
#define BUTTON_CONTROL_UP_WIDTH 85
#define BUTTON_CONTROL_UP_HEIGHT 45

/* Position of the control down button */
#define BUTTON_CONTROL_DOWN_X 207
#define BUTTON_CONTROL_DOWN_Y 396
#define BUTTON_CONTROL_DOWN_WIDTH 85
#define BUTTON_CONTROL_DOWN_HEIGHT 45

/* Position of the I32 button */
#define BUTTON_INST_32_X 281
#define BUTTON_INST_32_Y 198
#define BUTTON_INST_32_WIDTH 40
#define BUTTON_INST_32_HEIGHT 27

/* Position of the I24 button */
#define BUTTON_INST_24_X 319
#define BUTTON_INST_24_Y 236
#define BUTTON_INST_24_WIDTH 40
#define BUTTON_INST_24_HEIGHT 27

/* Position of the I16 button */
#define BUTTON_INST_16_X 326
#define BUTTON_INST_16_Y 283
#define BUTTON_INST_16_WIDTH 40
#define BUTTON_INST_16_HEIGHT 27

/* Position of the I8 button */
#define BUTTON_INST_8_X 312
#define BUTTON_INST_8_Y 331
#define BUTTON_INST_8_WIDTH 40
#define BUTTON_INST_8_HEIGHT 27

/* Position of the I0 button */
#define BUTTON_INST_0_X 279
#define BUTTON_INST_0_Y 366
#define BUTTON_INST_0_WIDTH 40
#define BUTTON_INST_0_HEIGHT 27

/* Position of the A32 button */
#define BUTTON_ACCUM_32_X 175
#define BUTTON_ACCUM_32_Y 366
#define BUTTON_ACCUM_32_WIDTH 40
#define BUTTON_ACCUM_32_HEIGHT 27

/* Position of the A24 button */
#define BUTTON_ACCUM_24_X 143
#define BUTTON_ACCUM_24_Y 331
#define BUTTON_ACCUM_24_WIDTH 40
#define BUTTON_ACCUM_24_HEIGHT 27

/* Position of the A16 button */
#define BUTTON_ACCUM_16_X 134
#define BUTTON_ACCUM_16_Y 284
#define BUTTON_ACCUM_16_WIDTH 40
#define BUTTON_ACCUM_16_HEIGHT 27

/* Position of the I8 button */
#define BUTTON_ACCUM_8_X 147
#define BUTTON_ACCUM_8_Y 236
#define BUTTON_ACCUM_8_WIDTH 40
#define BUTTON_ACCUM_8_HEIGHT 27

/* Position of the A0 button */
#define BUTTON_ACCUM_0_X 185
#define BUTTON_ACCUM_0_Y 198
#define BUTTON_ACCUM_0_WIDTH 40
#define BUTTON_ACCUM_0_HEIGHT 27

/* Background image */
extern const unsigned char front_panel_background_png[];
extern unsigned int front_panel_background_png_len;

/* Image with lit lamps for overlaying on the background */
extern const unsigned char front_panel_lamps_lit_png[];
extern unsigned int front_panel_lamps_lit_png_len;

/* Image with pressed button highlighting for overlaying on the background */
extern const unsigned char front_panel_buttons_pressed_png[];
extern unsigned int front_panel_buttons_pressed_png_len;

/* Main knob in the control up position */
extern const unsigned char front_panel_knob_control_up_png[];
extern unsigned int front_panel_knob_control_up_png_len;

/* Main knob in the control down position */
extern const unsigned char front_panel_knob_control_down_png[];
extern unsigned int front_panel_knob_control_down_png_len;

/* Main knob in the A0 position */
extern const unsigned char front_panel_knob_A_0_png[];
extern unsigned int front_panel_knob_A_0_png_len;

/* Main knob in the A8 position */
extern const unsigned char front_panel_knob_A_8_png[];
extern unsigned int front_panel_knob_A_8_png_len;

/* Main knob in the A16 position */
extern const unsigned char front_panel_knob_A_16_png[];
extern unsigned int front_panel_knob_A_16_png_len;

/* Main knob in the A24 position */
extern const unsigned char front_panel_knob_A_24_png[];
extern unsigned int front_panel_knob_A_24_png_len;

/* Main knob in the A32 position */
extern const unsigned char front_panel_knob_A_32_png[];
extern unsigned int front_panel_knob_A_32_png_len;

/* Main knob in the I0 position */
extern const unsigned char front_panel_knob_I_0_png[];
extern unsigned int front_panel_knob_I_0_png_len;

/* Main knob in the I8 position */
extern const unsigned char front_panel_knob_I_8_png[];
extern unsigned int front_panel_knob_I_8_png_len;

/* Main knob in the I16 position */
extern const unsigned char front_panel_knob_I_16_png[];
extern unsigned int front_panel_knob_I_16_png_len;

/* Main knob in the I24 position */
extern const unsigned char front_panel_knob_I_24_png[];
extern unsigned int front_panel_knob_I_24_png_len;

/* Main knob in the I32 position */
extern const unsigned char front_panel_knob_I_32_png[];
extern unsigned int front_panel_knob_I_32_png_len;

/* Font data */
extern const unsigned char ___fonts_DotMatrix_Bold_ttf[];
extern unsigned int ___fonts_DotMatrix_Bold_ttf_len;

#ifdef __cplusplus
}
#endif

#endif
