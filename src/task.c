#include "task.h"
#include "heap.h"
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
}

// Ring 3 (사용자 모드) 태스크 생성
void create_user_task(void (*entry)()) {
    if(num_tasks >= MAX_TASKS) return;
    
    task_t* task = &task_list[num_tasks];
    task->id = num_tasks;
    task->state = 0;
    task->stack = (uint64_t*)kmalloc(TASK_STACK_SIZE);
    
    uint64_t* stack_top = task->stack + (TASK_STACK_SIZE / 8);
    
    // 인터럽트 리턴(IRETQ)을 위한 가짜 레지스터 프레임
    *--stack_top = 0x20 | 3; // SS (Ring 3 Data Segment)
    *--stack_top = (uint64_t)task->stack + TASK_STACK_SIZE; // RSP
    *--stack_top = 0x202; // RFLAGS (인터럽트 활성화)
    *--stack_top = 0x18 | 3; // CS (Ring 3 Code Segment)
    *--stack_top = (uint64_t)entry; // RIP (사용자 앱 진입점)
    
    // 에러 코드와 인터럽트 번호
    *--stack_top = 0; // Error code
    *--stack_top = 0; // Interrupt number
    
    // 범용 레지스터 초기값
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
    
    regs->rsp = task_list[current_task].rsp;
}
