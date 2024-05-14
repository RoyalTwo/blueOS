; Declare constants for the multiboot header.
MBALIGN  equ  1 << 0            ; align loaded modules on page boundaries
MEMINFO  equ  1 << 1            ; provide memory map
MBFLAGS  equ  MBALIGN | MEMINFO ; this is the Multiboot 'flag' field
MAGIC    equ  0x1BADB002        ; 'magic number' lets bootloader find the header
CHECKSUM equ -(MAGIC + MBFLAGS)   ; checksum of above, to prove we are multiboot
 
section .multiboot
align 4
	dd MAGIC
	dd MBFLAGS
	dd CHECKSUM
 
; Stack grows downwards on x86. The stack must be 16-byte aligned
section .bss
align 16
stack_bottom:
resb 16384 ; 16 KiB
stack_top:
 
; The linker script specifies _start as the entry point to the kernel and the
; bootloader will jump to this position once the kernel has been loaded.
section .text
global _start:function (_start.end - _start)
_start:
    ; Setup stack
	mov ebp, stack_top
	mov esp, ebp

    ; Reset unimportant registers
    xor ecx, ecx
    mov es, bx
    mov fs, bx
    mov gs, bx
 
	; This is a good place to initialize crucial processor state.
	; Paging should be enabled here.
 
	extern kernel_main
	call kernel_main
 
	; infinite loop.
	cli
.hang:	hlt
	jmp .hang
.end:

global GDT_flush
extern gdt_ptr
GDT_flush:
	lgdt [gdt_ptr]
	mov ax, 0x10 ; 0x10 is data segment
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	jmp 0x08:flush2 ; 0x08 is code segment, this flushes registers
flush2:
	ret