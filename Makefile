GCC := compiler/bin/x86_64-elf-gcc
C_FLAGS := -ffreestanding -mno-red-zone -Wall -Wextra -Wpedantic -Wstrict-aliasing -nostdlib -nostdinc -fno-pie -fno-stack-protector -fno-builtin-function -fno-builtin
AS := nasm
AS_FLAGS := -f bin
KERNEL_DIR := src/kernel
BOOT_ASM_FILE := src/boot.asm
INCLUDE_DIR := src/kernel/include/
OUT_FILE := bin/blueOS.iso
TEMP_DIR := bin/temp

BOOT_BIN := $(TEMP_DIR)/boot.bin
KERNEL_BIN := $(TEMP_DIR)/kernel.bin

DISK_SIZE := 2880
# ^ num of blocks - 2880 * 512 bytes = about 1.48 MB like a floppy

clean:
	rm -rf $(TEMP_DIR)

# In the future, need to compile and link kernel files together as well
# For now, just compile one kernel file
build: $(BOOT_FILE) $(KERNEL_DIR)/kernel.c
	mkdir $(TEMP_DIR)
	$(AS) $(AS_FLAGS) $(BOOT_ASM_FILE) -o $(BOOT_BIN) 
	$(GCC) $(C_FLAGS) -I$(INCLUDE_DIR) $(KERNEL_DIR)/kernel.c -o $(KERNEL_BIN)
# Technically kernel is an ELF file but the extension seems interchangable

create-disk: build
	dd if=/dev/zero of=./$(OUT_FILE) bs=512 count=$(DISK_SIZE)
	dd if=$(BOOT_BIN) of=$(OUT_FILE) conv=notrunc bs=512 seek=0 count=1
	dd if=$(KERNEL_BIN) of=$(OUT_FILE) conv=notrunc bs=512 seek=1 count=2048
# count specifies how much to copy, for now just stay with 2048
