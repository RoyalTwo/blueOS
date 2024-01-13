bits 32
section .multiboot
	dd 0x1BADB002	; Magic number
	dd 0x0			; Flags
	dd - (0x1BADB002 + 0x0)	; Checksum

section .text
global start
extern main			; Defined in kernel.c

start:
	cli				; Disable interrupts
	mov esp, stack_space
	call main
	hlt
global GDT_flush
extern GDT_ptr
GDT_flush:
	lgdt [GDT_ptr]
	mov ax, 0x10	; 0x10 is data segment
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	jmp 0x08:flush2	; 0x08 is code segment, this flushes registers
flush2:
	ret

section .bss
align 16
resb 8192			; 8KB for stack, reserves space (NASM command)
stack_space: