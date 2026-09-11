// Copyright 2023 QMK
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include <string.h>
#include <stdio.h>

enum layers {
    _BASE,
    _INFO,
};

// custom_keycodes, key_descriptions[][], run_macro() and NUM_MACRO_LAYERS
// are generated from macros.json by generate_keymap.py -- edit that file,
// not this block.
#include "keymap_macros.h"

#ifdef OLED_ENABLE
// big_font[] (printable ASCII, 5 columns/glyph) is extracted from QMK's own
// drivers/oled/glcdfont.c by gen_bigfont.py -- see that script to regenerate.
#    include "keymap_bigfont.h"

// Draws `str` using big_font[], each pixel enlarged to a scale x scale
// block, horizontally centered on the display (vertically centered too,
// since every call here only ever draws one line).
static void oled_write_big(const char *str, uint8_t scale) {
    uint8_t len = strlen(str);
    uint8_t cell_w = 6 * scale; // 5 glyph columns + 1 space column
    int16_t x0 = ((int16_t)OLED_DISPLAY_WIDTH - (int16_t)(len * cell_w)) / 2;
    if (x0 < 0) {
        x0 = 0;
    }
    uint8_t y0 = (OLED_DISPLAY_HEIGHT - 8 * scale) / 2;

    for (uint8_t i = 0; i < len; i++) {
        uint8_t c = (uint8_t)str[i];
        if (c < BIG_FONT_FIRST || c > BIG_FONT_LAST) {
            c = ' ';
        }
        uint16_t glyph_off = (uint16_t)(c - BIG_FONT_FIRST) * 5;
        for (uint8_t col = 0; col < 5; col++) {
            uint8_t bits = pgm_read_byte(&big_font[glyph_off + col]);
            for (uint8_t row = 0; row < 8; row++) {
                if (!(bits & (1 << row))) {
                    continue;
                }
                for (uint8_t sx = 0; sx < scale; sx++) {
                    for (uint8_t sy = 0; sy < scale; sy++) {
                        oled_write_pixel((uint8_t)(x0 + i * cell_w + col * scale + sx), y0 + row * scale + sy, true);
                    }
                }
            }
        }
    }
}
#endif

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /*
     * ┌───┬───┬───┐
     * │ A │ B │ C │
     * ├───┼───┼───┤
     * │ D │ E │ F │  <- hold F to peek, tap for its own macro
     * └───┴───┴───┘
     * Hold A+B together (no need to also hold [i]) to cycle the macro layer.
     */
    [_BASE] = LAYOUT_ortho_2x3(
        MACRO_A, MACRO_B, MACRO_C,
        MACRO_D, MACRO_E, LT(_INFO, KC_F)
    ),

    // While F is held, the other keys don't fire their macros - they only
    // report their description to the OLED. Tapping F alone still sends KC_F.
    [_INFO] = LAYOUT_ortho_2x3(
        INFO_A,  INFO_B,  INFO_C,
        INFO_D,  INFO_E,  KC_TRNS
    ),
};

// Hold A+B together (on the base layer only -- while F/_INFO is held, A and B
// send INFO_A/INFO_B instead, so this combo can't fire mid-peek) to advance
// to the next macro layer.
const uint16_t PROGMEM      layer_combo[] = {MACRO_A, MACRO_B, COMBO_END};
combo_t                     key_combos[]  = {
    COMBO(layer_combo, CYCLE_LAYER),
};

// -1 = show the idle overview, >=0 = show that key's description.
static int8_t peek_index = -1;
// Which of macros.json's `layers` is active; A-E's actions and names both
// come from this row. Advanced by the CYCLE_LAYER combo, wraps around.
static uint8_t current_macro_layer = 0;

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (keycode >= INFO_A && keycode <= INFO_E) {
        peek_index = record->event.pressed ? (int8_t)(keycode - INFO_A) : -1;
        return false; // never send a HID report for these
    }
    if (keycode >= MACRO_A && keycode <= MACRO_E) {
        if (record->event.pressed) {
            run_macro(current_macro_layer, (uint8_t)(keycode - MACRO_A));
        }
        return false;
    }
    if (keycode == CYCLE_LAYER) {
        if (record->event.pressed) {
            current_macro_layer = (current_macro_layer + 1) % NUM_MACRO_LAYERS;
        }
        return false;
    }
    return true;
}

#ifdef OLED_ENABLE
bool oled_task_user(void) {
    // Only clear on an actual mode change (idle<->peek, switching which key
    // is peeked, or switching macro layer), not every frame: the peek
    // screen writes fewer lines than the idle screen, so without clearing
    // at all, whatever the idle screen left on the rows peek doesn't touch
    // stays behind. But clearing unconditionally on every call marks the
    // *whole* buffer dirty every frame, forcing a full-buffer retransmit
    // over I2C each time instead of only the bytes that changed -- on a
    // breadboard that's enough traffic to visibly glitch the display.
    static int8_t  last_peek_index = -2; // sentinel, unequal to any real state, forces the first clear
    static uint8_t last_macro_layer = 0xFF;
    if (peek_index != last_peek_index || current_macro_layer != last_macro_layer) {
        oled_clear();
        last_peek_index  = peek_index;
        last_macro_layer = current_macro_layer;
    }

    char layer_label[8];
    snprintf(layer_label, sizeof(layer_label), "L%u/%u", current_macro_layer + 1, NUM_MACRO_LAYERS);

    if (peek_index >= 0) {
        // Small corner label so you don't lose track of the layer mid-peek,
        // then the name itself, as large as reliably fits, dedicated screen.
        oled_set_cursor(0, 0);
        oled_write(layer_label, false);

        const char *name  = key_descriptions[current_macro_layer][peek_index];
        uint8_t     scale = strlen(name) <= 7 ? 3 : 2;
        oled_write_big(name, scale);
    } else {
        oled_write_ln(layer_label, false);
        oled_write_ln_P(PSTR(""), false);
        oled_write_ln_P(PSTR("A  B  C"), false);
        oled_write_ln_P(PSTR("D  E  [i]"), false);
        oled_write_ln_P(PSTR(""), false);
        oled_write_ln_P(PSTR("Hold [i] + key"), false);
        oled_write_ln_P(PSTR("to see what it does"), false);
    }
    return false;
}
#endif
