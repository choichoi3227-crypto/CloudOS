/* src/linker.ld */
ENTRY(_start)

SECTIONS
{
    /* 커널이 로드될 물리 주소 (보통 1MB) */
    . = 1M;

    /* Multiboot 헤더가 반드시 처음에 와야 함 */
    .text BLOCK(4K) : ALIGN(4K)
    {
        *(.multiboot)
        *(.text)
    }

    /* 읽기 전용 데이터 */
    .rodata BLOCK(4K) : ALIGN(4K)
    {
        *(.rodata)
    }

    /* 읽기/쓰기 데이터 */
    .data BLOCK(4K) : ALIGN(4K)
    {
        *(.data)
    }

    /* 초기화되지 않은 데이터 (BSS 섹션) */
    .bss BLOCK(4K) : ALIGN(4K)
    {
        *(COMMON)
        *(.bss)
        
        /* 커널 힙을 위한 메모리 공간 확보 */
        . = ALIGN(4096);
        __heap_start = .;
        . = . + 0x100000; /* 1MB 힙 할당 */
        __heap_end = .;
    }

    /* 커널 끝을 표시 (OOM 방지용) */
    __kernel_end = .;

    /* 버리는 섹션들 (크기 절감 및 에러 방지) */
    /DISCARD/ :
    {
        *(.comment)
        *(.eh_frame)
        *(.note.gnu.build-id)
    }
}
