CC = gcc
LD = ld
AS = nasm

CFLAGS = -ffreestanding -O2 -Wall -Wextra -mno-red-zone -m64 -fno-stack-protector -Isrc/include
LDFLAGS = -T linker.ld -m elf_x86_64 -nostdlib
ASFLAGS = -f elf64

# 10점 만점 모듈 추가
C_SOURCES = src/kernel.c src/graphics.c src/mouse.c src/compositor_pro.c src/idt.c src/keyboard.c src/string.c src/pmm.c src/vmm.c src/heap.c src/scheduler.c src/timer.c src/cloudfs_v3.c src/ahci.c src/power_acpi.c src/security_enhanced.c src/driver_framework.c src/app_subsystem.c
ASM_SOURCES = src/boot.asm src/interrupt.asm src/gdt.asm
OBJECTS = $(ASM_SOURCES:.asm=.o) $(C_SOURCES:.c=.o)

CloudOS.iso: CloudOS.bin
    mkdir -p isodir/boot/grub
    cp CloudOS.bin isodir/boot/
    echo 'set timeout=0' > isodir/boot/grub/grub.cfg
    echo 'set default=0' >> isodir/boot/grub/grub.cfg
    echo 'menuentry "CloudOS" {' >> isodir/boot/grub/grub.cfg
    echo '  multiboot2 /boot/CloudOS.bin' >> isodir/boot/grub/grub.cfg
    echo '  boot' >> isodir/boot/grub/grub.cfg
    echo '}' >> isodir/boot/grub/grub.cfg
    grub-mkrescue -o $@ isodir

CloudOS.bin: $(OBJECTS)
    $(LD) $(LDFLAGS) -o $@ $(OBJECTS)

%.o: %.asm
    $(AS) $(ASFLAGS) $< -o $@

%.o: %.c
    $(CC) $(CFLAGS) -c $< -o $@

clean:
    rm -f $(OBJECTS) CloudOS.bin CloudOS.iso
    rm -rf isodir

run: CloudOS.iso
    qemu-system-x86_64 -cdrom CloudOS.iso -m 512M -drive file=disk.img,format=raw,id=disk,if=none -device ide-hd,drive=disk
