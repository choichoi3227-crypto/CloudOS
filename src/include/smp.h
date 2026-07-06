#ifndef SMP_H
#define SMP_H
#include "types.h"

void smp_init(void);
void smp_schedule(int cpu_id, uint64_t* rsp);
void smp_create_task(void (*entry)(), int cpu_affinity);

#endif
