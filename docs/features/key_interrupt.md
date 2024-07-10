# Key Interrupt

Upon a selected key press down, if another key is pressed, it will release another key. This is similar to the "Kill switch" function created by Realforce.

## How do I enable key interrupt {#how-do-i-enable-key-interrupt}

In `info.json` add this:
```json
"features" : {
    // ... other features
    "key_interrupt" : true
}
```

**OR**

in `rules.mk` add this:
```make
KEY_INTERRUPT_ENABLE = yes
```

By default, key interrupt is disabled. To enable it, you need to use the `KI_TOGG` or `KI_ON` keycode to enable it. The status is stored in persistent memory, so you shouldn't need to enable it again.

### Keycodes {#keycodes}

|Keycode                  |Aliases  |Description                                     |
|-------------------------|---------|------------------------------------------------|
|`QK_KEY_INTERRUPT_ON`    |`KI_ON`  |Turns on the key interrupt feature.             |
|`QK_KEY_INTERRUPT_OFF`   |`KI_OFF` |Turns off the key interrupt feature.            |
|`QK_KEY_INTERRUPT_TOGGLE`|`KI_TOGG`|Toggles the status of the key interrupt feature.|

## User Callback Functions

```c
bool process_key_interrupt_user(uint16_t keycode, keyrecord_t *record);
```

## Process Key Interrupt Example

```c
const key_interrupt_t PROGMEM key_interrupt_list[] = {
    // on key down
    //       |    key to be released
    //       |     |
    [0] = {KC_D, KC_A},
    [1] = {KC_A, KC_D},
    [2] = {KC_A, KC_F},
};
```

## Key Interrupt Status

Additional user callback functions to manipulate Key Interrupt:

| Function                     | Description                                    |
|------------------------------|------------------------------------------------|
| `key_interrupt_enable()`     | Turns Key Interrupt on.                        |
| `key_interrupt_disable()`    | Turns Key Interrupt off.                       |
| `key_interrupt_toggle()`     | Toggles Key Interrupt.                         |
| `key_interrupt_is_enabled()` | Returns true if Key Interrupt is currently on. |
