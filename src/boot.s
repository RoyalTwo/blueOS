; Declare constants for the multiboot header.
MBALIGN  equ  1 << 0            ; align loaded modules on page boundaries
MEMINFO  equ  1 << 1            ; provide memory map
MBFLAGS  equ  MBALIGN | MEMINFO ; this is the Multiboot 'flag' field
MAGIC    equ  0x1BADB002        ; 'magic number' lets bootloader find the header
CHECKSUM equ -(MAGIC + MBFLAGS)   ; checksum of above, to prove we are multiboot
 
; Declare a multiboot header that marks the program as a kernel. The bootloader will
; search for this signature in the first 8 KiB of the kernel file, aligned at a
; 32-bit boundary. The signature is in its own section so the header can be
; forced to be within the first 8 KiB of the kernel file.
section .multiboot
align 4
	dd MAGIC
	dd MBFLAGS
	dd CHECKSUM
 
; Allocates room for a
; small stack by creating a symbol at the bottom of it, then allocating 16384
; bytes for it, and finally creating a symbol at the top. The stack grows
; downwards on x86. The stack must be 16-byte aligned
section .bss
align 16
stack_bottom:
resb 16384 ; 16 KiB
stack_top:
 
; The linker script specifies _start as the entry point to the kernel and the
; bootloader will jump to this position once the kernel has been loaded.
; Declare _start as a function symbol with the given symbol size
section .text
global _start:function (_start.end - _start)
_start:
    ; Setup stack
	mov esp, stack_top

    ; Reset unimportant registers
    xor ecx, ecx
    mov es, bx
    mov fs, bx
    mov gs, bx
 
	; This is a good place to initialize crucial processor state before the
	; high-level kernel is entered. The GDT should be loaded here. Paging should be enabled here.
 
	extern kernel_main
	call kernel_main
 
	; infinite loop.
	cli
.hang:	hlt
	jmp .hang
.end: