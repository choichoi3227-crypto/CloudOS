#ifndef SYNC_H
#define SYNC_H
#include "types.h"

// 실제 하드웨어 락 (xchg 명령어 사용)
typedef struct {
    volatile int locked;
} spinlock_t;

// 뮤텍스 (스핀락 기반)
typedef struct {
    spinlock_t lock;
    volatile int locked;
    int owner_pid;
} mutex_t;

// 세마포어
typedef struct {
    spinlock_t lock;
    volatile int count;
} semaphore_t;

void spinlock_init(spinlock_t* lock);
void spinlock_acquire(spinlock_t* lock);
void spinlock_release(spinlock_t* lock);

void mutex_init(mutex_t* m);
void mutex_acquire(mutex_t* m, int pid);
void mutex_release(mutex_t* m);

void sem_init(semaphore_t* s, int count);
void sem_wait(semaphore_t* s);
void sem_signal(semaphore_t* s);

#endif
