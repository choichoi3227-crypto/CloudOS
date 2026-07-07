/* src/linker.ld */
OUTPUT_FORMAT(elf32-i386) /* 이 한 줄이 64비트/32비트 충돌을 막아줍니다 */
ENTRY(_start)

SECTIONS
{
    . = 1M;
    .text BLOCK(4K) : ALIGN(4K) { *(.multiboot) *(.text) }
    .rodata BLOCK(4K) : ALIGN(4K) { *(.rodata) }
    .data BLOCK(4K) : ALIGN(4K) { *(.data) }
    .bss BLOCK(4K) : ALIGN(4K) {
        *(COMMON)
        *(.bss)
        . = ALIGN(4096);
        __heap_start = .;
        . = . + 0x100000;
        __heap_end = .;
    }
    /DISCARD/ : { *(.comment) *(.eh_frame) }
}
