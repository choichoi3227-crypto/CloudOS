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
    task_list[0].pml4 = 0;
}

void create_user_task(void (*entry)()) {
    create_user_task_with_pml4((uint64_t)entry, 0);
}

void create_user_task_with_pml4(uint64_t entry, uint64_t pml4) {
    if(num_tasks >= MAX_TASKS) return;
    
    task_t* task = &task_list[num_tasks];
    task->id = num_tasks;
    task->state = 0;
    task->pml4 = pml4;
    task->stack = (uint64_t*)kmalloc(TASK_STACK_SIZE);
    
    uint64_t* stack_top = task->stack + (TASK_STACK_SIZE / 8);
    
    *--stack_top = 0x20 | 3;
    *--stack_top = (uint64_t)task->stack + TASK_STACK_SIZE;
    *--stack_top = 0x202;
    *--stack_top = 0x18 | 3;
    *--stack_top = entry;
    
    *--stack_top = 0;
    *--stack_top = 0;
    
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
    
    if (task_list[current_task].pml4 != 0) {
        __asm__ __volatile__("mov %0, %%cr3" : : "r"(task_list[current_task].pml4));
    }
    
    regs->rsp = task_list[current_task].rsp;
}
