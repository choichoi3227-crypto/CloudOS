# src/Makefile
CROSS_COMPILE = i686-elf- # 32비트 커널용 크로스 컴파일러 (없으면 gcc로 변경)
CC = $(CROSS_COMPILE)gcc
AS = $(CROSS_COMPILE)nasm
LD = $(CROSS_COMPILE)ld

CFLAGS = -m32 -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -nostdinc -Iinclude
ASFLAGS = -f elf32
LDFLAGS = -m elf_i386 -T linker.ld

# 소스 파일 수집
C_SOURCES = $(wildcard *.c) $(wildcard */*.c)
ASM_SOURCES = $(wildcard *.asm)
C_OBJECTS = $(C_SOURCES:.c=.o)
ASM_OBJECTS = $(ASM_SOURCES:.asm=.o)
ALL_OBJECTS = $(C_OBJECTS) $(ASM_OBJECTS)

TARGET = ../CloudOS.bin

.PHONY: all clean

all: $(TARGET)

 $(TARGET): $(ALL_OBJECTS)
    $(LD) $(LDFLAGS) -o $@ $^
    @echo "[BUILD SUCCESS] OS Image created: $(TARGET)"

%.o: %.c
    $(CC) $(CFLAGS) -c $< -o $@

%.o: %.asm
    $(AS) $(ASFLAGS) $< -o $@

clean:
    rm -f $(ALL_OBJECTS) $(TARGET)
