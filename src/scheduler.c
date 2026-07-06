#include "scheduler.h"
#include "vmm.h"
#include "heap.h"
#include "string.h"

static process_t process_table[MAX_PROCESSES];
static int current_pid = 0;
static int num_processes = 0;

void scheduler_init(void) {
    memset(process_table, 0, sizeof(process_table));
    current_pid = 0;
    num_processes = 1;
    
    // Kernel Idle Process
    process_table[0].pid = 0;
    process_table[0].priority = TASK_IDLE;
    process_table[0].pml4 = 0; // 커널 PML4 사용
}

int create_process(void (*entry)(), task_priority_t prio) {
    if (num_processes >= MAX_PROCESSES) return -1;
    
    process_t* p = &process_table[num_processes];
    p->pid = num_processes;
    p->priority = prio;
    p->cpu_affinity = 0; // TODO: NUMA 인식 스케줄링
    p->pml4 = vmm_create_user_pml4();
    
    // 스택 및 컨텍스트 설정 (생략: 기존 task.c 로직 활용)
    
    num_processes++;
    return p->pid;
}

void schedule(uint64_t* rsp) {
    process_table[current_pid].rsp = *rsp;
    process_table[current_pid].cpu_time_used++;
    
    // 폴리모픽 스케줄링: 우선순위가 높은 태스크 먼저 실행
    int best_pid = 0;
    task_priority_t best_prio = TASK_IDLE;
    
    for (int i = 1; i < num_processes; i++) {
        if (process_table[i].priority < best_prio) {
            best_prio = process_table[i].priority;
            best_pid = i;
        }
    }
    
    // 백그라운드 프로세스 쓰로틀링 (CPU 점유율 제한)
    if (best_prio == TASK_BACKGROUND && process_table[best_pid].cpu_time_used % 10 != 0) {
        best_pid = 0; // Idle 프로세스 실행으로 CPU 휴식
    }
    
    current_pid = best_pid;
    *rsp = process_table[current_pid].rsp;
    
    // 주소 공간 전환
    if (process_table[current_pid].pml4 != 0) {
        __asm__ __volatile__("mov %0, %%cr3" : : "r"(process_table[current_pid].pml4));
    }
}
