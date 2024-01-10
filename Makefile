BOOT=src/boot/boot.asm
KERNEL=src/kernel/kernel.c
ISOFILE=bin/glados.iso
ISO_VOLUME_NAME=GLADOS
LINKER=src/linker.ld
KERNEL_OUT=bin/glados.bin

all: build
build: clean
	mkdir -p bin
	nasm -f elf32 ${BOOT} -o bin/boot.o
	gcc -m32 -ffreestanding -c ${KERNEL} -o bin/kernel.o
	ld -m elf_i386 -T ${LINKER} -o ${KERNEL_OUT} bin/boot.o bin/kernel.o
run: build 
	qemu-system-i386 -kernel ${KERNEL_OUT} -monitor stdio
iso: build 
	mkdir -p bin/iso/boot/grub
	cp grub.cfg bin/iso/boot/grub
	cp ${KERNEL_OUT} bin/iso/boot/grub
	grub-mkrescue -o bin/glados.iso bin/iso
	rm -rf bin/iso
run-iso: iso
	qemu-system-i386 -cdrom ${ISOFILE} 
clean:
	rm -rf build