BOOT=src/boot/boot.asm
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
	nasm -f elf32 $(KERNEL_DIR)/idt_asm.asm -o $(OBJ_DIR)/idt_asm.o
	$(COMPILER) -ffreestanding -g -nostdlib -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mno-3dnow -T $(LINKER) $(OBJ_DIR)/boot.o $(OBJ_DIR)/idt_asm.o $^  -lgcc -o $(KERNEL_OUT) 
$(OBJ_DIR)/%.o: $(KERNEL_DIR)/%.c 
	$(COMPILER) -ffreestanding -g -I$(KERNEL_HEADER_DIR) -c $< -o $@ 
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
debug-iso: iso
	qemu-system-i386 -s -S -cdrom $(ISOFILE)
# have to keep or it removes objs
clean:
	rm -rf build