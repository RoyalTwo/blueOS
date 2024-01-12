BOOT=src/boot/boot.asm
KERNEL_DIR=src/kernel
KERNEL_HEADER_DIR=src/kernel/lib
ISOFILE=bin/glados.iso
LINKER=src/linker.ld
KERNEL_OUT=bin/glados.bin
COMPILER=i686-elf-gcc
KERNEL_DIR=src/kernel
KERNEL_FILES:=$(wildcard $(KERNEL_DIR)/*.c)
OBJ_DIR=bin/objs
OBJ_FILES=$(patsubst $(KERNEL_DIR)/%.c, $(OBJ_DIR)/%.o, $(KERNEL_FILES))

all: build 
build: $(OBJ_FILES)
	nasm -f elf32 ${BOOT} -o $(OBJ_DIR)/boot.o
	$(COMPILER) -ffreestanding -nostdlib -T $(LINKER) $^ $(OBJ_DIR)/boot.o -lgcc -o $(KERNEL_OUT) 
$(OBJ_DIR)/%.o: $(KERNEL_DIR)/%.c
	$(COMPILER) -ffreestanding -I$(KERNEL_HEADER_DIR) -c $< -o $@ 
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
# have to keep or it removes objs
clean:
	rm -rf build