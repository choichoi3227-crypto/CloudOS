#include "smp.h"
#include "vmm.h"
#include "apic.h"

#define MAX_CPUS 8

typedef struct {
    uint64_t rsp;
    uint64_t pml4;
    int pid;
    int cpu_affinity; // -1이면 모든 CPU, 특정 번호면 해당 CPU만
} smp_task_t;

static smp_task_t runqueues[MAX_CPUS][64]; // CPU별 런큐
static int current_pid_per_cpu[MAX_CPUS];

void smp_init(void) {
    for (int i = 0; i < MAX_CPUS; i++) {
        current_pid_per_cpu[i] = -1;
    }
    // APIC를 통해 모든 코어에 SIPI(Startup IPI) 전송하여 활성화
    apic_send_init_ipi();
}

void smp_schedule(int cpu_id, uint64_t* rsp) {
    // 현재 CPU의 런큐에서 다음 태스크 선택 (Round-Robin + Affinity)
    int next_pid = (current_pid_per_cpu[cpu_id] + 1) % 64;
    
    // Affinity 확인
    while (runqueues[cpu_id][next_pid].pid == 0 || 
           (runqueues[cpu_id][next_pid].cpu_affinity != -1 && 
            runqueues[cpu_id][next_pid].cpu_affinity != cpu_id)) {
        next_pid = (next_pid + 1) % 64;
    }

    if (current_pid_per_cpu[cpu_id] != -1) {
        runqueues[cpu_id][current_pid_per_cpu[cpu_id]].rsp = *rsp;
    }

    current_pid_per_cpu[cpu_id] = next_pid;
    *rsp = runqueues[cpu_id][next_pid].rsp;

    // 컨텍스트 스위칭 (CR3 교체)
    if (runqueues[cpu_id][next_pid].pml4 != 0) {
        __asm__ __volatile__("mov %0, %%cr3" : : "r"(runqueues[cpu_id][next_pid].pml4));
    }
}
