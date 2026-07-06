#include "sync.h"

void spinlock_init(spinlock_t* lock) {
    lock->locked = 0;
}

void mutex_init(mutex_t* m) {
    spinlock_init(&m->lock);
    m->locked = 0;
    m->owner_pid = 0;
}

void sem_init(semaphore_t* s, int count) {
    spinlock_init(&s->lock);
    s->count = count;
}

// x86_64 xchg 명령어를 이용한 원자적 락 획득
void spinlock_acquire(spinlock_t* lock) {
    __asm__ __volatile__(
        "1:\n"
        "lock bts $0, %0\n"     // 비트 0을 테스트하고 설정 (원자적)
        "jc 2f\n"               // 이미 잠겨있으면 2로 점프
        "ret\n"                 // 락 획득 성공, 리턴
        "2:\n"
        "pause\n"               // CPU 파이프라인 정리 (하이퍼스레딩 효율)
        "jmp 1b\n"              // 다시 시도
        : "+m" (lock->locked)
        :
        : "memory", "cc"
    );
}

void spinlock_release(spinlock_t* lock) {
    __asm__ __volatile__(
        "movl $0, %0\n"
        : "+m" (lock->locked)
        :
        : "memory"
    );
}

void mutex_acquire(mutex_t* m, int pid) {
    spinlock_acquire(&m->lock);
    while (m->locked) {
        spinlock_release(&m->lock);
        __asm__ __volatile__("pause");
        spinlock_acquire(&m->lock);
    }
    m->locked = 1;
    m->owner_pid = pid;
    spinlock_release(&m->lock);
}

void mutex_release(mutex_t* m) {
    spinlock_acquire(&m->lock);
    m->locked = 0;
    m->owner_pid = 0;
    spinlock_release(&m->lock);
}

void sem_wait(semaphore_t* s) {
    spinlock_acquire(&s->lock);
    while (s->count <= 0) {
        spinlock_release(&s->lock);
        __asm__ __volatile__("pause");
        spinlock_acquire(&s->lock);
    }
    s->count--;
    spinlock_release(&s->lock);
}

void sem_signal(semaphore_t* s) {
    spinlock_acquire(&s->lock);
    s->count++;
    spinlock_release(&s->lock);
}
