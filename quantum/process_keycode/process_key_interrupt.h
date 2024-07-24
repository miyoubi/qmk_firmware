// Copyright 2024 Harrison Chan (@xelus22)

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "action.h"

#define NUM_INTERRUPT_KEYCODES 2

typedef struct key_interrupt_t {
    uint16_t press, unpress;
} key_interrupt_t;

bool process_key_interrupt(uint16_t keycode, keyrecord_t *record);
bool process_key_interrupt_user(uint16_t keycode, keyrecord_t *record);

bool key_interrupt_recovery_is_enabled(void);
void key_interrupt_recovery_enable(void);
void key_interrupt_recovery_disable(void);
void key_interrupt_recovery_toggle(void);

bool key_interrupt_is_enabled(void);
void key_interrupt_enable(void);
void key_interrupt_disable(void);
void key_interrupt_toggle(void);
