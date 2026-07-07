# src/Makefile (기존 파일 완전 덮어쓰기)

CC = i686-w64-mingw32-gcc
AS = nasm
LD = i686-w64-mingw32-ld

# ==========================================
# [핵심 수정 1] -fno-pic 추가 (상용 OS 커널 필수 옵션)
# PIC(Position Independent Code)는 공유 라이브러리(.so)를 위해 만들어지는 코드입니다.
# OS 커널은 고정 주소에 로드되므로 PIC가 필요 없고, 있으면 오히려 GOT를 찾지 못해 죽습니다.
# ==========================================
CFLAGS = -m32 -ffreestanding -fno-pic -fno-builtin -fno-stack-protector -nostdlib -Iinclude -w -fpermissive
ASFLAGS = -f elf32
LDFLAGS = -m elf_i386 -T linker.ld --oformat binary -nostdlib

# ==========================================
# [핵심 수정 2] 블랙리스트 정규식 업데이트
# 경로가 없는(^\.\/) 파일들만 격리하여, 하위 폴더의 정상적인 파일(mm/heap.c 등)은 보호함
# ==========================================
DEAD_CODE_REGEX = net/tcp\.c|kernel/smp\.c|kernel/syscall\.c|sys/update\.c|apps/browser\.c|pkg/cloudpkg\.c
LEGACY_REGEX = ^\.\/(heap\.c|cloudfs\.c|cloudfs_v3\.c|task\.c)

# 정상 소스 수집
C_SOURCES = $(shell find . -name "*.c" | grep -v -E "$(DEAD_CODE_REGEX)" | grep -v -E "$(LEGACY_REGEX)")
ASM_SOURCES = $(shell find . -name "*.asm")
ALL_OBJECTS = $(patsubst %.c, %.o, $(C_SOURCES)) $(patsubst %.asm, %.o, $(ASM_SOURCES))

TARGET = ../CloudOS.bin

.PHONY: all clean

all: $(TARGET)
    @echo "=============================================="
    @echo " [BUILD SUCCESS] CloudOS Built Securely!"
    @echo " Compiled: $(words $(ALL_OBJECTS)) core files"
    @echo " Excluded: Dead code & Legacy duplicates"
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
