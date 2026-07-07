# src/Makefile (기존 파일 완전 덮어쓰기)

# 크로스 컴파일러 설정 (없으면 그냥 gcc 사용)
CROSS_COMPILE = i686-elf-
CC = $(CROSS_COMPILE)gcc
AS = nasm
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy

# 상용 OS 커널 컴파일 필수 플래그 (경고 무시, 최적화, 프리스탠딩)
CFLAGS = -m32 -ffreestanding -fno-builtin -fno-stack-protector -nostdlib -nostdinc -Iinclude -w -fpermissive
ASFLAGS = -f elf32
LDFLAGS = -m elf_i386 -T linker.ld --oformat binary

# ==========================================
# 핵심 마법: 하위 폴더까지 모두 자동 수집 (경로 오류 제로)
# ==========================================
C_SOURCES = $(shell find . -name "*.c")
ASM_SOURCES = $(shell find . -name "*.asm")

C_OBJECTS = $(patsubst %.c, %.o, $(C_SOURCES))
ASM_OBJECTS = $(patsubst %.asm, %.o, $(ASM_SOURCES))
ALL_OBJECTS = $(ASM_OBJECTS) $(C_OBJECTS)

TARGET = ../CloudOS.bin

.PHONY: all clean list

all: $(TARGET)
    @echo ""
    @echo "=============================================="
    @echo " [BUILD SUCCESS] CloudOS Kernel Built!"
    @echo " Target: $(TARGET)"
    @echo " Objects: $(words $(ALL_OBJECTS)) files compiled"
    @echo "=============================================="

 $(TARGET): $(ALL_OBJECTS)
    @echo "[LINKER] Linking kernel binary..."
    $(LD) $(LDFLAGS) -o $@ $^

# C 파일 컴파일 규칙
%.o: %.c
    @echo "[CC] $<"
    $(CC) $(CFLAGS) -c $< -o $@

# ASM 파일 컴파일 규칙
%.o: %.asm
    @echo "[ASM] $<"
    $(AS) $(ASFLAGS) $< -o $@

clean:
    @echo "[CLEAN] Removing object files..."
    $(shell find . -name "*.o" -delete)
    rm -f $(TARGET)

# 현재 빌드 대상 파일들을 보여주는 디버깅 명령어
list:
    @echo "=== C Sources ==="
    @echo $(C_SOURCES) | tr ' ' '\n'
    @echo "=== ASM Sources ==="
    @echo $(ASM_SOURCES) | tr ' ' '\n'
