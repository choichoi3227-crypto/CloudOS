#ifndef SCHEDULER_H
#define SCHEDULER_H
#include "types.h"

#define MAX_PROCESSES 64
#define MAX_CPU_CORES 8

typedef enum {
    TASK_REALTIME = 0,   // 실시간 (오디오, 입력)
    TASK_INTERACTIVE = 1,// UI 앱 포그라운드
    TASK_BACKGROUND = 2, // 백그라운드 동기화
    TASK_IDLE = 3        // 시스템 유휴
} task_priority_t;

typedef struct {
    uint64_t rsp;
    uint64_t pml4;
    int pid;
    task_priority_t priority;
    uint64_t cpu_time_used;
    int cpu_affinity; // 특정 코어에 바인딩 (NUMA/SMP 최적화)
} process_t;

void scheduler_init(void);
int create_process(void (*entry)(), task_priority_t prio);
void schedule(uint64_t* rsp);

#endif
