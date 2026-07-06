#ifndef KEYBOARD_H
#define KEYBOARD_H
#include "types.h"
void keyboard_init(void);
void keyboard_handle_scancode(uint8_t scancode);
char keyboard_getchar(void);
#endif
