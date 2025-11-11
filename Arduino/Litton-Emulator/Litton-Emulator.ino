
#include "litton/litton.h"

litton_state_t machine;
litton_device_t printer;
litton_device_t keyboard;
litton_device_t tape_reader;
litton_device_t tape_punch;
bool tape_reader_on = false;

#define KEYBUF_SIZE 256 // Must be a power of 2.
unsigned char keybuf[KEYBUF_SIZE];
unsigned keybuf_in = 0;
unsigned keybuf_out = 0;
unsigned keybuf_size = 0;

static void printer_output
    (litton_state_t *state, litton_device_t *device,
     uint8_t value, litton_parity_t parity)
{
    if (state->printer_charset == LITTON_CHARSET_EBS1231 &&
            parity == LITTON_PARITY_NONE) {
        // Sometimes OPUS outputs a character with "OI" or "OA"
        // that already has the parity bit set.  Strip it off.
        value = litton_remove_parity(value, LITTON_PARITY_ODD);
    }
    if (state->printer_charset == LITTON_CHARSET_EBS1231) {
        // Does this look like a print wheel position?
        uint8_t position = litton_print_wheel_position(value);
        if (position != 0) {
            // Yes, so space forward or backspace back to put the
            // print head in the right column.
            --position;
            while (device->print_position < position) {
                Serial.print(' ');
                ++(device->print_position);
            }
            while (device->print_position > position) {
                Serial.print('\b');
                --(device->print_position);
            }
        } else if (value == 075 || value == 055 || value == 054) {
            // Line Feed Left / Line Feed Right / Line Feed Both.
            Serial.print('\n');
        } else {
            // Convert the code into its ASCII form.
            const char *string_form;
            int ch = litton_char_from_charset
                (value, state->printer_charset, &string_form);
            if (ch == '\f') {
                // Form feed; just output a carriage return and line feed.
                Serial.println();
                device->print_position = 0;
            } else if (ch == '\r') {
                // Carriage return.
                Serial.print((char)ch);
                device->print_position = 0;
            } else if (ch >= 0) {
                // Single character with a direct ASCII mapping.
                Serial.print((char)ch);
                ++(device->print_position);
            }
        }
    } else {
        // Assume plain ASCII.
        Serial.print((char)value);
    }
}

static int keyboard_input
    (litton_state_t *state, litton_device_t *device,
     uint8_t *value, litton_parity_t parity)
{
    if (keybuf_size == 0) {
        // Nothing to do if there is no input available.
        return 0;
    }
    int ch = keybuf[keybuf_out++];
    keybuf_out &= (KEYBUF_SIZE - 1);
    --keybuf_size;
    size_t posn = 0;
    char chbuf = (char)ch;
    int ch2 = litton_char_map_special(ch);
    if (ch2 >= 0) {
        // Special function key.
        *value = litton_add_parity(ch2, parity);
        return 1;
    }
    ch2 = litton_char_to_charset(&chbuf, &posn, 1, device->charset);
    if (ch2 >= 0) {
        // We have a valid character in the keyboard's character set.
        *value = litton_add_parity(ch2, parity);
        return 1;
    }
    return 0;
}

static int tape_reader_input
    (litton_state_t *state, litton_device_t *device,
     uint8_t *value, litton_parity_t parity)
{
    if (keyboard_input(state, device, value, parity)) {
        // Convert LF into CR to deal with pasted tape images,
        // but only if we didn't already output a CR.
        if (*value == 075 && !(device->print_position)) {
            *value = 0100;
        } else if (*value == 0100) {
            device->print_position = 1;
        } else {
            device->print_position = 0;
        }

        // Echo the character to give the user feedback as
        // to the progress of tape loading.
        if (*value == 0100) {
            Serial.println();
        } else {
            printer_output(state, device, *value, parity);
        }
        return 1;
    } else {
        return 0;
    }
}

void setup()
{
    // Initialize the serial port and print the banner.
    Serial.begin(115200);
    Serial.println();
    Serial.println("Litton EBS1231 Emulator");
    Serial.println("Press '?' for help, CTRL-C to halt.");
    Serial.println();

    // Initialize the machine.
    litton_init(&machine);
    litton_load_opus(&machine);

    // Create the standard keyboard and printer devices.
    keyboard.id = LITTON_DEVICE_KEYBOARD;
    keyboard.supports_input = 1;
    keyboard.charset = LITTON_CHARSET_EBS1231;
    keyboard.input = keyboard_input;
    litton_add_device(&machine, &keyboard);
    printer.id = LITTON_DEVICE_PRINTER;
    printer.supports_output = 1;
    printer.charset = LITTON_CHARSET_EBS1231;
    printer.output = printer_output;
    litton_add_device(&machine, &printer);

    // Tape reader and punch are mapped to the keyboard and printer.
    tape_reader.id = LITTON_DEVICE_READER;
    tape_reader.supports_input = 1;
    tape_reader.charset = LITTON_CHARSET_EBS1231;
    tape_reader.input = tape_reader_input;
    litton_add_device(&machine, &tape_reader);
    tape_punch.id = LITTON_DEVICE_PUNCH;
    tape_punch.supports_output = 1;
    tape_punch.charset = LITTON_CHARSET_EBS1231;
    tape_punch.output = printer_output;
    litton_add_device(&machine, &tape_punch);

    // Reset the machine.
    litton_reset(&machine);

    // Press HALT, READY, and then RUN to start running OPUS.
    litton_press_button(&machine, LITTON_BUTTON_HALT);
    litton_press_button(&machine, LITTON_BUTTON_READY);
    litton_press_button(&machine, LITTON_BUTTON_RUN);

    // Ready to go.
    Serial.println("READY");
}

void print_help()
{
    Serial.println();
    Serial.println("Litton      ASCII       Meaning");
    Serial.println("----------------------------------------------------------------");
    Serial.println("[I]         CTRL-W      End of command");
    Serial.println("[II]        CTRL-E");
    Serial.println("[III]       CTRL-R");
    Serial.println("[IIII]      CTRL-T      Run program");
    Serial.println("[P1]        CTRL-Y      Read program tape");
    Serial.println("[P2]        CTRL-U      Verify program tape");
    Serial.println("[P3]        CTRL-O");
    Serial.println("[P4]        CTRL-P");
    Serial.println("SHIFT+[P4]  CTRL-K      Confirm R command");
    Serial.println();
    Serial.println("OPUS Commands");
    Serial.println("----------------------------------------------------------------");
    Serial.println("H           Select hexadecimal format");
    Serial.println("J n#        Jump to native program at address n (hex mode only)");
    Serial.println("K           Print the native accumulator and carry values");
    Serial.println("L           Punch tape leader");
    Serial.println("N           Select decimal format");
    Serial.println("O           Select octal format");
    Serial.println("P           Select program registers");
    Serial.println("Q n#        Change register n");
    Serial.println("R           Reset program memory");
    Serial.println("S n#        Store instructions or data starting at n");
    Serial.println("T           Write program to tape");
    Serial.println("V           Select variable registers");
    Serial.println("W n#m#      Print registers n to m");
    Serial.println("X           Print the A register");
    Serial.println("Y           Print the B register");
    Serial.println();
}

void loop()
{
    litton_step_result_t step;
    int ch;

    // Copy character input to "keybuf".
    while ((ch = Serial.read()) >= 0) {
        if (ch == 0x03) {
            // Force a halt if the user pressed CTRL-C.
            litton_press_button(&machine, LITTON_BUTTON_HALT);

            // Clear the input buffer on CTRL-C.
            keybuf_in = keybuf_out;
            keybuf_size = 0;
            break;
        } else if (ch == '?') {
            print_help();
        } else if (keybuf_size < KEYBUF_SIZE) {
            keybuf[keybuf_in++] = (unsigned char)ch;
            keybuf_in &= (KEYBUF_SIZE - 1);
            ++keybuf_size;
        }
    }

    // Step the machine by one instruction.
    if (litton_is_halted(&machine)) {
        step = LITTON_STEP_HALT;
    } else {
        step = litton_step(&machine);
    }
    if (step != LITTON_STEP_OK) {
        // Machine has halted.  Print "HALT" and then restart OPUS.
        Serial.println("HALT");
        printer.print_position = 0;
        litton_press_button(&machine, LITTON_BUTTON_HALT);
        litton_press_button(&machine, LITTON_BUTTON_READY);
        litton_press_button(&machine, LITTON_BUTTON_RUN);
    }

    // Report when the tape reader is turned on or off to let the
    // user known when a tape image can be pasted in.
    bool reader_on = tape_reader.selected != 0;
    if (tape_reader_on != reader_on) {
        Serial.println();
        Serial.print("TAPE READER O");
        if (reader_on)
            Serial.println("N");
        else
            Serial.println("FF");
    }
    tape_reader_on = reader_on;
}
