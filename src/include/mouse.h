#ifndef MOUSE_H
#define MOUSE_H
#include "types.h"

void mouse_init(void);
void mouse_handler(void);
int mouse_get_x(void);
int mouse_get_y(void);
int mouse_get_btn(void);

#endif
