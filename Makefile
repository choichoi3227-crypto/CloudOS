CC = gcc
LD = ld
AS = nasm
CFLAGS = -ffreestanding -O0 -Wall -Wextra -mno-red-zone -m64 -fno-stack-protector -Isrc/include
LDFLAGS = -T linker.ld -m elf_x86_64 -nostdlib
ASFLAGS = -f elf64
C_SOURCES = src/kernel.c src/vga.c src/idt.c src/keyboard.c src/string.c src/pmm.c src/ide.c src/cloudfs_core.c \
	src/acpi.c src/ahci.c src/app_subsystem.c src/audio.c src/browser.c src/cloudfs_advanced.c \
	src/cloudoptimizer.c src/compositor.c src/compositor_pro.c src/driver_framework.c src/e1000.c \
	src/elf.c src/gpu.c src/graphics.c src/heap.c src/mouse.c src/net.c src/power_acpi.c \
	src/power_manager.c src/security.c src/security_enhanced.c src/subsystem.c src/sync.c src/task.c \
	src/timer.c src/usb.c src/vfs.c src/vmm.c src/wm.c \
	src/drivers/nvme.c src/fs/cloudfs_v4.c src/mm/slab.c src/net/tcp.c
ASM_SOURCES = src/boot.asm src/interrupt.asm
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
	qemu-system-x86_64 -cdrom CloudOS.iso -m 256M -drive file=disk.img,format=raw,if=ide
