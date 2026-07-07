# Remove built-in rules and variables
override MAKEFLAGS += -rR

# Name of final executable
override KERNEL := blueOS

# A cross compiler is required. Since I am currently building on MacOS, I installed one with Homebrew.
# This will need to be changed with Windows though, to use a cross compiler not within the PATH.
# TODO:
override KCC := x86_64-elf-gcc

# Changeable C flags
override KCFLAGS := -g -O2 -pipe

# Changeable C preprocessor flags
override KCPPFLAGS :=

# Changeable NASM flags
override KNASM_FLAGS := -F dwarf -g

# Changeable linker flags
override KLDFLAGS :=

# Include directory for kernel. TODO: Use more robust system for folder separation
override INCLUDE_DIR := src/include/

# Internal C flags, should NOT be changed
override KCFLAGS += \
	-Wall \
	-Wextra \
	-std=gnu11 \
	-ffreestanding \
	-fno-stack-protector \
	-fno-stack-check \
	-fPIE \
	-m64 \
	-march=x86-64 \
	-mno-80387 \
	-mno-mmx \
	-mno-sse \
	-mno-sse2 \
	-mno-red-zone

# Internal C preprocessor flags, should NOT be changed
override KCPPFLAGS := \
	-I $(INCLUDE_DIR) \
	$(KCPPFLAGS) \
	-MMD \
	-MP

# Internal linker flags, should NOT be changed
override KLDFLAGS += \
	-nostdlib \
	-Wl,-m,elf_x86_64 \
	-Wl,-nostdlib \
	-Wl,-pie \
	-Wl,-z,text \
	-Wl,-z,max-page-size=0x1000 \
	-Wl,-T,linker.ld

# Internal NASM flags, should NOT be changed
override KNASMFLAGS += \
	-Wall \
	-f elf64

# Uses "find" to glob all *.c, *.S, and *.asm files in the tree and obtain the
# object and header dependency file names
override CFILES := $(shell cd src && find -L * -type f -name '*.c')
override ASFILES := $(shell cd src && find -L * -type f -name '*.S')
override NASMFILES := $(shell cd src && find -L * -type f -name '*.asm')
override OBJ := $(addprefix obj/,$(CFILES:.c=.c.o) $(ASFILES:.S=.S.o) $(NASMFILES:.asm=.asm.o))
override HEADER_DEPS := $(addprefix obj/,$(CFILES:.c=.c.d) $(ASFILES:.S=.S.d))

# Default target
.PHONY: all
all: bin/$(KERNEL)

src/include/limine.h:
	curl -Lo $@ https://github.com/limine-bootloader/limine/raw/trunk/limine.h

run:
	./CreateISO.sh
	qemu-system-x86_64 -serial stdio -cdrom bin/blueOS.iso

# Link rules for the final kernel executable.
# The magic printf/dd command is used to force the final ELF file type to ET_DYN.
# GNU binutils currently forces the ELF type to ET_EXEC even for
# relocatable PIEs, if the base load address is non-0.
# See https://sourceware.org/bugzilla/show_bug.cgi?id=31795 for more information
bin/$(KERNEL): GNUmakefile linker.ld $(OBJ)
	mkdir -p "$$(dirname $@)"
	$(KCC) $(KCFLAGS) $(OBJ) $(KLDFLAGS) -o $@
	printf '\003' | dd of=$@ bs=1 count=1 seek=16 conv=notrunc 2>/dev/null

# Include header dependencies.
-include $(HEADER_DEPS)

# Compilation rules for *.c files
obj/%.c.o: src/%.c GNUmakefile src/include/limine.h
	mkdir -p "$$(dirname $@)"
	$(KCC) $(KCFLAGS) $(KCPPFLAGS) -c $< -o $@

# Compilation rules for *.S files
obj/%.S.o: src/%.S GNUmakefile
	mkdir -p "$$(dirname $@)"
	$(KCC) $(KCFLAGS) $(KCPPFLAGS) -c $< -o $@

# Compilation rules for *.asm (nasm) files
obj/%.asm.o: src/%.asm GNUmakefile
	mkdir -p "$$(dirname $@)"
	nasm $(KNASMFLAGS) $< -o $@

# Remove object files and the final executable
.PHONY: clean
clean:
	rm -rf bin obj iso_root limine

.PHONY: distclean
distclean: clean
	rm -f src/limine.h