/* Copyright 2022 Jason Wihardja
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include QMK_KEYBOARD_H

#define IDLE_TIMEOUT_MS 3000

enum custom_keycodes {
    MOUSE_STOP = SAFE_RANGE,
    MOUSE_MOVE_HORIZONTAL,
    MOUSE_MOVE_VERTICAL
};

uint16_t keycode_1, keycode_2;
static deferred_token idle_token = INVALID_DEFERRED_TOKEN;

static uint32_t move_mouse_callback(uint32_t trigger_time, void* cb_arg) {
    tap_code(keycode_1);
    wait_ms(1000);
    tap_code(keycode_2);
    return IDLE_TIMEOUT_MS;
}

void set_idle_token_if_not_extended() {
    if (!extend_deferred_exec(idle_token, IDLE_TIMEOUT_MS)) {
        idle_token = defer_exec(IDLE_TIMEOUT_MS, move_mouse_callback, NULL);
    }
}

void set_mouse_horizontal_direction() {
    keycode_1 = KC_MS_LEFT;
    keycode_2 = KC_MS_RIGHT;
}

void set_mouse_vertical_direction() {
    keycode_1 = KC_MS_UP;
    keycode_2 = KC_MS_DOWN;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case MOUSE_STOP:
            if (record->event.pressed) {
                cancel_deferred_exec(idle_token);
            }
            return false;
            break;
        case MOUSE_MOVE_HORIZONTAL:
            if (record->event.pressed) {
                set_mouse_horizontal_direction();
                set_idle_token_if_not_extended();
            }
            return false;
            break;
        case MOUSE_MOVE_VERTICAL:
            if (record->event.pressed) {
                set_mouse_vertical_direction();
                set_idle_token_if_not_extended();
            }
            return false;
            break;
        }
    return true;
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    [0] = LAYOUT(MOUSE_STOP, MOUSE_MOVE_HORIZONTAL, MOUSE_MOVE_VERTICAL),

};
