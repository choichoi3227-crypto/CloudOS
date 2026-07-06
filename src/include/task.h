#ifndef TASK_H
#define TASK_H
#include "types.h"

#define MAX_TASKS 10
#define TASK_STACK_SIZE 4096

struct registers {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rdi, rsi, rbp, rdx, rcx, rbx, rax;
    uint64_t int_no, err_code;
    uint64_t rip, cs, rflags, rsp, ss;
};

typedef struct task {
    uint64_t rsp;
    uint64_t* stack;
    uint64_t pml4;
    int id;
    int state;
} task_t;

void task_init(void);
void create_user_task(void (*entry)());
void create_user_task_with_pml4(uint64_t entry, uint64_t pml4);
void schedule(struct registers* regs);

#endif
