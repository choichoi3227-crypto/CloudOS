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

void create_task(void (*entry)()) {
    if(num_tasks >= MAX_TASKS) return;
    
    task_t* task = &task_list[num_tasks];
    task->id = num_tasks;
    task->state = 0;
    task->stack = (uint64_t*)kmalloc(TASK_STACK_SIZE);
    
    uint64_t* stack_top = task->stack + (TASK_STACK_SIZE / 8);
    
    // 가짜 레지스터 프레임 생성 (IRETQ를 위해)
    *--stack_top = 0x10; // SS
    *--stack_top = (uint64_t)task->stack + TASK_STACK_SIZE; // RSP
    *--stack_top = 0x202; // RFLAGS
    *--stack_top = 0x08; // CS
    *--stack_top = (uint64_t)entry; // RIP
    
    // 레지스터 초기값
    for(int i=0; i<15; i++) *--stack_top = 0;
    
    task->rsp = (uint64_t)stack_top;
    num_tasks++;
}

void schedule(struct registers* regs) {
    if(num_tasks <= 1) return;
    
    // 현재 태스크 상태 저장
    task_list[current_task].rsp = regs->rsp; // 인터럽트 발생 시 스택 포인터
    
    // 다음 태스크 선택 (라운드 로빈)
    task_list[current_task].state = 0;
    current_task = (current_task + 1) % num_tasks;
    task_list[current_task].state = 1;
    
    // 다음 태스크 스택으로 전환
    regs->rsp = task_list[current_task].rsp;
}
