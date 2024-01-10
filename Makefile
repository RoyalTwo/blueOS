ASM=nasm

SRC_DIR=src
OUT_DIR=bin
BOOT_SRC_DIR=$(SRC_DIR)/boot
KERNEL_SRC_SIR=$(SRC_DIR)/kernel

image: $(OUT_DIR)/main.img

$(OUT_DIR)/main.img: bootloader kernel
	dd if=/dev/zero of=$(OUT_DIR)/image.img bs=512 count=2880
	mkfs.fat -F 12 -n "LOS" $(OUT_DIR)/image.img
	dd if=$(OUT_DIR)/bootloader.bin of=$(OUT_DIR)/image.img conv=notrunc
	mcopy -i $(OUT_DIR)/image.img $(OUT_DIR)/kernel.bin "::kernel.bin"

bootloader: $(OUT_DIR)/bootloader.bin
kernel: $(OUT_DIR)/kernel.bin

$(OUT_DIR)/bootloader.bin: $(BOOT_SRC_DIR)/bootloader.asm
	$(ASM) $(BOOT_SRC_DIR)/bootloader.asm -f bin -o $(OUT_DIR)/bootloader.bin

$(OUT_DIR)/kernel.bin: $(KERNEL_SRC_SIR)/kernel.asm
	$(ASM) $(KERNEL_SRC_SIR)/kernel.asm -f bin -o $(OUT_DIR)/kernel.bin

clean:
	rm -rf $(OUT_DIR)/*