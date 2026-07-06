#ifndef CLOUDOPTIMIZER_H
#define CLOUDOPTIMIZER_H
#include "types.h"

void optimizer_init(void);
void optimizer_run(void); // 주기적 호출을 통한 발열/배터리 최적화
void optimizer_set_dirty_rect(int x, int y, int w, int h);
int optimizer_is_dirty(int x, int y);

#endif
