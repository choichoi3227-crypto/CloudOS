CC = gcc
LD = ld
AS = nasm

CFLAGS = -ffreestanding -O0 -Wall -Wextra -mno-red-zone -m64 -fno-stack-protector -Isrc/include
LDFLAGS = -T linker.ld -m elf_x86_64 -nostdlib
ASFLAGS = -f elf64

# audio.c 추가
C_SOURCES = src/kernel.c src/graphics.c src/mouse.c src/wm.c src/usb.c src/acpi.c src/audio.c src/idt.c src/keyboard.c src/string.c src/pmm.c src/vmm.c src/heap.c src/task.c src/timer.c src/vfs.c src/cloudfs.c src/ahci.c src/elf.c src/e1000.c src/net.c
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
    qemu-system-x86_64 -cdrom CloudOS.iso -m 512M -drive file=disk.img,format=raw,id=disk,if=none -device ide-hd,drive=disk -netdev user,id=net0 -device e1000,netdev=net0
