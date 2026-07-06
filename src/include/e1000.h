#ifndef E1000_H
#define E1000_H
#include "types.h"
#include "net.h"

void e1000_init(void);
void e1000_handler(void);
void e1000_send_packet(void* data, uint32_t len);

#endif
