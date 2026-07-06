CC = gcc
LD = ld
AS = nasm

CFLAGS = -ffreestanding -O2 -Wall -Wextra -mno-red-zone -m64 -fno-stack-protector -Isrc/include
LDFLAGS = -T linker.ld -m elf_x86_64 -nostdlib
ASFLAGS = -f elf64

# 모듈형 디렉토리 구조 반영
C_SOURCES = src/kernel/kernel.c src/kernel/syscall.c src/net/tcp.c src/pkg/cloudpkg.c src/sys/update.c \
            src/gui/graphics.c src/gui/wm.c src/drivers/mouse.c src/drivers/ahci.c \
            src/fs/cloudfs_v4.c src/mm/pmm.c src/mm/vmm.c src/mm/slab.c \
            src/arch/x86_64/idt.c src/arch/x86_64/gdt.c src/lib/string.c
ASM_SOURCES = src/arch/x86_64/boot.asm src/arch/x86_64/interrupt.asm
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
    qemu-system-x86_64 -cdrom CloudOS.iso -m 512M -drive file=disk.img,format=raw,id=disk,if=none -device ide-hd,drive=disk -netdev user,id=net0 -device e1000,netdev=net0
