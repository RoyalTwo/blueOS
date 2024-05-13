GCC := compiler/bin/i686-elf-gcc
C_BUILD_FLAGS := -ffreestanding -mno-red-zone -O2 -Wall -Wextra -fno-stack-protector -fno-builtin-function -fno-builtin
C_LINK_FLAGS := -T linker.ld -ffreestanding -O2 -nostdlib -lgcc
AS := nasm
AS_FLAGS := -felf32

KERNEL_DIR := src/kernel
BOOT_FILE := src/boot.s
INCLUDE_DIR := src/kernel/include/
TEMP_DIR := bin/temp

OUT_BIN_FILE := bin/temp/blueOS.bin
OUT_ISO_FILE := bin/blueOS.iso

SOURCES := $(wildcard $(KERNEL_DIR)/*.c)
OBJECTS := $(patsubst $(KERNEL_DIR)/%.c, $(TEMP_DIR)/%.o, $(SOURCES))
OBJECTS_WITH_BOOT := $(TEMP_DIR)/boot.o $(OBJECTS)


clean:
	rm -rf $(TEMP_DIR)
	mkdir $(TEMP_DIR)

$(TEMP_DIR)/%.o: $(KERNEL_DIR)/%.c
	$(GCC) $(C_BUILD_FLAGS) -I$(INCLUDE_DIR) -c $< -o $@

$(TEMP_DIR)/boot.o: $(BOOT_FILE)
	$(AS) $(AS_FLAGS) $(BOOT_FILE) -o $(TEMP_DIR)/boot.o

build: $(OBJECTS_WITH_BOOT)
	$(GCC) $(C_LINK_FLAGS) -o $(OUT_BIN_FILE) $^

iso: build
	cp $(OUT_BIN_FILE) isodir/boot/blueOS.bin
	grub-mkrescue -o $(OUT_ISO_FILE) isodir

qemu: iso
	qemu-system-i386 -hda $(OUT_ISO_FILE)
