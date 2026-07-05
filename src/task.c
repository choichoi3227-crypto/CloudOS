#include "task.h"
#include "heap.h"
#include "vmm.h"
#include "vga.h"
#include "string.h"

static task_t task_list[MAX_TASKS];
static int num_tasks = 0;
static int current_task = 0;

void task_init(void) {
    num_tasks = 1;
    task_list[0].id = 0;
    task_list[0].state = 1;
    task_list[0].stack = NULL;
    task_list[0].pml4 = 0; // 커널 메인 태스크는 0 (추후 VMM에서 가져오도록 개선 필요)
}

void create_user_task(void (*entry)()) {
    // 기본 커널 PML4 사용 (간단한 테스트용)
    create_user_task_with_pml4((uint64_t)entry, 0);
}

void create_user_task_with_pml4(uint64_t entry, uint64_t pml4) {
    if(num_tasks >= MAX_TASKS) return;
    
    task_t* task = &task_list[num_tasks];
    task->id = num_tasks;
    task->state = 0;
    task->pml4 = pml4; // 독립 PML4 설정
    task->stack = (uint64_t*)kmalloc(TASK_STACK_SIZE);
    
    uint64_t* stack_top = task->stack + (TASK_STACK_SIZE / 8);
    
    // IRETQ 프레임
    *--stack_top = 0x20 | 3; // SS (Ring 3)
    *--stack_top = (uint64_t)task->stack + TASK_STACK_SIZE; // RSP
    *--stack_top = 0x202; // RFLAGS
    *--stack_top = 0x18 | 3; // CS (Ring 3)
    *--stack_top = entry; // RIP
    
    *--stack_top = 0; // Error code
    *--stack_top = 0; // Interrupt number
    
    for(int i=0; i<15; i++) *--stack_top = 0;
    
    task->rsp = (uint64_t)stack_top;
    num_tasks++;
}

void schedule(struct registers* regs) {
    if(num_tasks <= 1) return;
    
    task_list[current_task].rsp = regs->rsp;
    task_list[current_task].state = 0;
    
    current_task = (current_task + 1) % num_tasks;
    task_list[current_task].state = 1;
    
    // 페이지 테이블 전환 (CR3 업데이트)
    if (task_list[current_task].pml4 != 0) {
        __asm__ __volatile__("mov %0, %%cr3" : : "r"(task_list[current_task].pml4));
    }
    
    regs->rsp = task_list[current_task].rsp;
}
