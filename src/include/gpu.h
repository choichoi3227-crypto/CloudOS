// gpu.h
#ifndef GPU_H
#define GPU_H

#include "types.h"

void gpu_init(void);

// 아래 세 개는 하드웨어 가속 연동을 위한 인터페이스
int gpu_is_available(void);                                 // GPU 사용 가능 여부 반환
void gpu_blit_to_framebuffer(const void *src, size_t size); // 백버퍼→프론트버퍼 블릿
void gpu_fill_rect(int x, int y, int w, int h, uint32_t color); // 단순 2D Fill 스텁

#endif
