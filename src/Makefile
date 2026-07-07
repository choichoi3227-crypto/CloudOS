# src/Makefile (이전 것을 완전히 덮어쓰으세요)

# MSYS2에서 설치한 32비트 툴체인 사용 (i686 = 32비트)
CC = i686-w64-mingw32-gcc
AS = nasm
LD = i686-w64-mingw32-ld

# 32비트 OS 전용 컴파일 옵션 (64비트 코드가 단 하나도 들어가지 않음)
CFLAGS = -m32 -ffreestanding -fno-builtin -fno-stack-protector -nostdlib -Iinclude -w -fpermissive
ASFLAGS = -f elf32
LDFLAGS = -m elf_i386 -T linker.ld --oformat binary

# 모든 하위 폴더의 소스 자동 수집
C_SOURCES = $(shell find . -name "*.c")
ASM_SOURCES = $(shell find . -name "*.asm")
ALL_OBJECTS = $(patsubst %.c, %.o, $(C_SOURCES)) $(patsubst %.asm, %.o, $(ASM_SOURCES))

TARGET = ../CloudOS.bin

.PHONY: all clean

all: $(TARGET)
    @echo "=============================================="
    @echo " [BUILD SUCCESS] 32-bit CloudOS Built!"
    @echo "=============================================="

 $(TARGET): $(ALL_OBJECTS)
    $(LD) $(LDFLAGS) -o $@ $^

%.o: %.c
    $(CC) $(CFLAGS) -c $< -o $@

%.o: %.asm
    $(AS) $(ASFLAGS) $< -o $@

clean:
    find . -name "*.o" -delete
    rm -f $(TARGET)
