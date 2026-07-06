#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H
#include "types.h"

void power_manager_init(void);
void power_manager_throttle_background(void);
void power_manager_enter_deep_sleep(void);

#endif
