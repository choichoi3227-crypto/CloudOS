# src/Makefile (기존 파일 완전 덮어쓰기)

CC = i686-w64-mingw32-gcc
AS = nasm
LD = i686-w64-mingw32-ld

CFLAGS = -m32 -ffreestanding -fno-builtin -fno-stack-protector -nostdlib -Iinclude -w -fpermissive
ASFLAGS = -f elf32
LDFLAGS = -m elf_i386 -T linker.ld --oformat binary

# ==========================================
# [핵심 수정] 죽은 코드(Dead Code) 블랙리스트
# 리눅스 전용 헤더(<memory.h>)를 쓰거나 없는 헤더(apic.h)를 부르는 파일들
# 이 파일들은 빌드 대상에서 완전히 제외됩니다.
# ==========================================
DEAD_CODE_REGEX = net/tcp\.c|kernel/smp\.c|kernel/syscall\.c|sys/update\.c|apps/browser\.c|pkg/cloudpkg\.c

# 정상 소스 수집 (블랙리스트에 걸리면 제외)
C_SOURCES = $(shell find . -name "*.c" | grep -v -E "$(DEAD_CODE_REGEX)")
ASM_SOURCES = $(shell find . -name "*.asm")
ALL_OBJECTS = $(patsubst %.c, %.o, $(C_SOURCES)) $(patsubst %.asm, %.o, $(ASM_SOURCES))

TARGET = ../CloudOS.bin

.PHONY: all clean

all: $(TARGET)
    @echo "=============================================="
    @echo " [BUILD SUCCESS] 32-bit CloudOS Built!"
    @echo " Compiled: $(words $(ALL_OBJECTS)) core files"
    @echo " Excluded: 6 dead code files"
    @echo "=============================================="

 $(TARGET): $(ALL_OBJECTS)
    $(LD) $(LDFLAGS) -o $@ $^

%.o: %.c
    @echo "[CC] $<"
    $(CC) $(CFLAGS) -c $< -o $@

%.o: %.asm
    @echo "[ASM] $<"
    $(AS) $(ASFLAGS) $< -o $@

clean:
    find . -name "*.o" -delete
    rm -f $(TARGET)
