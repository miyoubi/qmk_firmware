/* Copyright 2024 Harrison Chan (@xelus22)
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

#include "process_key_interrupt.h"
#include <string.h>
#include "keycodes.h"
#include "keycode_config.h"
#include "action_util.h"
#include "keymap_introspection.h"
#include "debug.h"

#define KEYREPORT_BUFFER_SIZE 10

// key interrupt up stroke buffer
uint16_t buffer_keyreports[KEYREPORT_BUFFER_SIZE];

// is the next free one
int buffer_keyreport_count = 0;

// temp buffer to store keycodes
uint16_t buffer_keyreports_temp[KEYREPORT_BUFFER_SIZE];

/**
 * @brief function for querying the enabled state of key interrupt
 *
 * @return true if enabled
 * @return false if disabled
 */
bool key_interrupt_is_enabled(void) {
    return keymap_config.key_interrupt_enable;
}

/**
 * @brief Enables key interrupt and saves state to eeprom
 *
 */
void key_interrupt_enable(void) {
    keymap_config.key_interrupt_enable = true;
    eeconfig_update_keymap(keymap_config.raw);
}

/**
 * @brief Disables key interrupt and saves state to eeprom
 *
 */
void key_interrupt_disable(void) {
    keymap_config.key_interrupt_enable = false;
    eeconfig_update_keymap(keymap_config.raw);
}

/**
 * @brief Toggles key interrupt's status and save state to eeprom
 *
 */
void key_interrupt_toggle(void) {
    keymap_config.key_interrupt_enable = !keymap_config.key_interrupt_enable;
    eeconfig_update_keymap(keymap_config.raw);
}

/**
 * @brief function for querying the enabled state of key interrupt recovery
 *
 * @return true if enabled
 * @return false if disabled
 */
bool key_interrupt_recovery_is_enabled(void) {
    // should check both
    return keymap_config.key_interrupt_enable && keymap_config.key_interrupt_recovery_enable;
}

/**
 * @brief Enables key interrupt recovery and saves state to eeprom
 *
 */
void key_interrupt_recovery_enable(void) {
    keymap_config.key_interrupt_recovery_enable = true;
    eeconfig_update_keymap(keymap_config.raw);
}

/**
 * @brief Disables key interrupt recovery and saves state to eeprom
 *
 */
void key_interrupt_recovery_disable(void) {
    keymap_config.key_interrupt_recovery_enable = false;
    eeconfig_update_keymap(keymap_config.raw);
}

/**
 * @brief Toggles key interrupt recovery's status and save state to eeprom
 *
 */
void key_interrupt_recovery_toggle(void) {
    keymap_config.key_interrupt_recovery_enable = !keymap_config.key_interrupt_recovery_enable;
    eeconfig_update_keymap(keymap_config.raw);
}

/**
 * @brief handler for user to override whether autocorrect should process this keypress
 *
 * @param keycode Keycode registered by matrix press, per keymap
 * @param record keyrecord_t structure
 * @return true Allow key interrupt
 * @return false Stop processing and escape from key interrupt
 */
__attribute__((weak)) bool process_key_interrupt_user(uint16_t keycode, keyrecord_t *record) {
    return true;
}

// check if key already in buffer
bool key_interrupt_is_key_in_buffer(uint16_t keycode) {
    for (int i = 0; i < buffer_keyreport_count; i++) {
        if (buffer_keyreports[i] == keycode) {
            return true;
        }
    }
    return false;
}

void add_key_buffer(uint16_t keycode) {
    if (key_interrupt_is_key_in_buffer(keycode)) {
        return;
    }

    if (buffer_keyreport_count >= KEYREPORT_BUFFER_SIZE) {
        return;
    }

    buffer_keyreports[buffer_keyreport_count] = keycode;
    buffer_keyreport_count++;
}

// remove keycode and shift buffer
void del_key_buffer(uint16_t keycode) {
    for (int i = 0; i < buffer_keyreport_count; i++) {
        if (buffer_keyreports[i] == keycode) {
            for (int j = i; j < buffer_keyreport_count - 1; j++) {
                buffer_keyreports[j] = buffer_keyreports[j + 1];
            }
            buffer_keyreport_count--;
            break;
        }
    }
}

// remove keycode, dont care about shifting buffer
void del_key_buffer_temp(uint16_t keycode, int end_index) {
    for (int i = 0; i < end_index; i++) {
        if (buffer_keyreports_temp[i] == keycode) {
            buffer_keyreports_temp[i] = 0;
            break;
        }
    }
}

// check if keycode is in the interrupt press list
bool key_interrupt_is_key_in_press_list(uint16_t keycode) {
    for (int i = 0; i < key_interrupt_count(); i++) {
        if (key_interrupt_get(i).press == keycode) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Process handler for key_interrupt feature
 *
 * @param keycode Keycode registered by matrix press, per keymap
 * @param record keyrecord_t structure
 * @return true Continue processing keycodes, and send to host
 * @return false Stop processing keycodes, and don't send to host
 */
bool process_key_interrupt(uint16_t keycode, keyrecord_t *record) {
    if ((keycode >= QK_KEY_INTERRUPT_ON && keycode <= QK_KEY_INTERRUPT_RECOVERY_TOGGLE) && record->event.pressed) {
        if (keycode == QK_KEY_INTERRUPT_ON) {
            key_interrupt_enable();
        } else if (keycode == QK_KEY_INTERRUPT_OFF) {
            key_interrupt_disable();
        } else if (keycode == QK_KEY_INTERRUPT_TOGGLE) {
            key_interrupt_toggle();
        } else if (keycode == QK_KEY_INTERRUPT_RECOVERY_ON) {
            key_interrupt_recovery_enable();
        } else if (keycode == QK_KEY_INTERRUPT_RECOVERY_OFF) {
            key_interrupt_recovery_disable();
        } else if (keycode == QK_KEY_INTERRUPT_RECOVERY_TOGGLE) {
            key_interrupt_recovery_toggle();
        } else {
            return true;
        }

        return false;
    }

    if (!keymap_config.key_interrupt_enable) {
        return true;
    }

    // only supports basic keycodes
    if (!IS_BASIC_KEYCODE(keycode)) {
        return true;
    }

    // custom user hook
    if (!process_key_interrupt_user(keycode, record)) {
        return true;
    }

    // if key interrupt recovery is not enabled then do not process key up events
    if (!keymap_config.key_interrupt_recovery_enable && !record->event.pressed) {
        return true;
    }

    // only key interrupt to buffer if required
    if (keymap_config.key_interrupt_recovery_enable) {
        if (key_interrupt_is_key_in_press_list(keycode)) {
            if (record->event.pressed) {
                add_key_buffer(keycode);
            } else {
                del_key_buffer(keycode);
            }
        }

        if (buffer_keyreport_count == 0) {
            return true;
        }
    }

    // no key interrupt to process

    // copy buffer to temp buffer
    memcpy(buffer_keyreports_temp, buffer_keyreports, sizeof(buffer_keyreports));

    ac_dprintf("buffer_keyreport_count: %d\n", buffer_keyreport_count);

    if (record->event.pressed) {
        for (int i = 0; i < key_interrupt_count(); i++) {
            key_interrupt_t key_interrupt = key_interrupt_get(i);
            if (keycode == key_interrupt.press) {
                del_key(key_interrupt.unpress);
            }
        }
    } else {
        for (int j = buffer_keyreport_count - 1; j >= 0; j--) {
            for (int i = 0; i < key_interrupt_count(); i++) {
                key_interrupt_t key_interrupt = key_interrupt_get(i);
                // if key in buffer is the same as the key interrupt press
                if (key_interrupt.press == buffer_keyreports_temp[j]) {
                    // if key interrupt unpress is in buffer
                    if (key_interrupt_is_key_in_buffer(key_interrupt.unpress)) {
                        // remove key interrupt unpress from buffer
                        del_key_buffer_temp(key_interrupt.unpress, j);
                    }
                }
            }
        }

        // print temp buffer
        for (int i = 0; i < buffer_keyreport_count; i++) {
            ac_dprintf("buffer_keyreports_temp[%d]: %d\n", i, buffer_keyreports_temp[i]);
        }

        // print buffer
        for (int i = 0; i < buffer_keyreport_count; i++) {
            ac_dprintf("buffer_keyreports[%d]: %d\n", i, buffer_keyreports[i]);
        }

        // compare buffer and temp buffer
        for (int i = 0; i < buffer_keyreport_count; i++) {
            if (buffer_keyreports[i] != buffer_keyreports_temp[i]) {
                if (buffer_keyreports_temp[i] == 0) {
                    del_key(buffer_keyreports[i]);
                }
            } else {
                add_key(buffer_keyreports_temp[i]);
            }
        }
    }

    return true;
}
