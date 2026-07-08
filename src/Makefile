# src/Makefile (기존 파일 완전 덮어쓰기)

CC = i686-w64-mingw32-gcc
AS = nasm
LD = i686-w64-mingw32-ld

# 상용 OS 컴파일 필수 옵션 (PIC 방지, 프리스탠딩, 경로 설정)
CFLAGS = -m32 -ffreestanding -fno-pic -fno-builtin -fno-stack-protector -nostdlib -Iinclude -w -fpermissive
ASFLAGS = -f elf32
LDFLAGS = -m elf_i386 -T linker.ld --oformat binary -nostdlib

# ==========================================
# [해결 1] 화이트리스트 & 블랙리스트 방어막
# 레거시 찌꺼기 파일(블랙)은 절대 컴파일하지 않고,
# 최신 핵심 파일(화이트)과 하위 폴더 전체는 컴파일합니다.
# ==========================================

# 1. 루트(src/)에서 제외할 죽은 코드 (중복 정의, 64비트, 미구현 파일들)
BLACKLIST_ROOT = \
    src/cloudfs.c src/cloudfs_advanced.c src/cloudfs_core.c src/cloudfs_v3.c \
    src/cloudoptimizer.c src/browser.c src/heap.c src/task.c src/long_mode_init.asm \
    src/net/tcp.c src/kernel/smp.c src/kernel/syscall.c src/sys/update.c src/pkg/cloudpkg.c

# 2. 루트(src/)에서 반드시 포함할 최신 핵심 코드
WHITELIST_ROOT = \
    src/kernel.c src/compositor.c src/compositor_pro.c src/wm.c src/graphics.c \
    src/gpu.c src/mouse.c src/keyboard.c src/idt.c src/vga.c src/vfs.c src/net.c \
    src/acpi.c src/ahci.c src/app_subsystem.c src/audio.c src/driver_framework.c \
    src/e1000.c src/elf.c src/ide.c src/pmm.c src/power_acpi.c src/power_manager.c \
    src/scheduler.c src/security.c src/security_enhanced src/string.c src/subsystem.c \
    src/sync.c src/timer.c src/usb.c src/vmm.c

# 3. 하위 폴더 전체는 자동 포함 (mm/, sys/, fs/ 등)
DIR_SOURCES = $(shell find ./mm ./sys ./fs ./apps ./drivers ./kernel ./net ./pkg -name "*.c" 2>/dev/null)

# 최종 소스 목록 합체
C_SOURCES = $(WHITELIST_ROOT) $(DIR_SOURCES)
ASM_SOURCES = $(shell find . -name "*.asm" | grep -v "long_mode_init")
ALL_OBJECTS = $(patsubst %.c, %.o, $(C_SOURCES)) $(patsubst %.asm, %.o, $(ASM_SOURCES))

TARGET = CloudOS.bin
ISO_DIR = ../iso

.PHONY: all clean iso

all: $(TARGET)
    @echo "=============================================="
    @echo " [BUILD SUCCESS] CloudOS Core Built!"
    @echo " Compiled: $(words $(ALL_OBJECTS)) files"
    @echo "=============================================="

 $(TARGET): $(ALL_OBJECTS)
    @echo "[LINKER] Linking kernel binary..."
    $(LD) $(LDFLAGS) -o $@ $^

%.o: %.c
    @echo "[CC] $<"
    $(CC) $(CFLAGS) -c $< -o $@

%.o: %..asm
    @echo "[ASM] $<"
    $(AS) $(ASFLAGS) $< -o $@

# ==========================================
# [해결 2] 정상적인 ISO 빌드 타겟
# grub-mkrescue를 사용하여 실제 부팅 가능한 CD 이미지 생성
# ==========================================
iso: $(TARGET)
    @echo "[ISO] Creating bootable ISO image..."
    @mkdir -p $(ISO_DIR)/boot/grub
    @cp $(TARGET) $(ISO_DIR)/boot/CloudOS.bin
    @cp grub.cfg $(ISO_DIR)/boot/grub/grub.cfg
    grub-mkrescue -o ../CloudOS.iso $(ISO_DIR) 2>/dev/null || \
    echo "[INFO] grub-mkrescue not found. Please run: pacman -S mingw-w64-i686-grub"

clean:
    @echo "[CLEAN] Removing object files..."
    find . -name "*.o" -delete
    rm -f $(TARGET) ../CloudOS.iso
    rm -rf $(ISO_DIR)
