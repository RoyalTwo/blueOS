bits 64

global gdt_flush
global tss_load
gdt_flush:
    lgdt [rdi]
    ; Reload CS register:
    PUSH 0x08                 ; Push code segment to stack, 0x08 is a stand-in for your code segment
    LEA RAX, [rel .reload_CS] ; Load address of .reload_CS into RAX
    PUSH RAX                  ; Push this value to the stack
    RETFQ                     ; Perform a far return, RETFQ or LRETQ depending on syntax
.reload_CS:
    ; Reload data segment registers
    MOV   AX, 0x10 ; 0x10 is a stand-in for your data segment
    MOV   DS, AX
    MOV   ES, AX
    MOV   FS, AX
    MOV   GS, AX
    MOV   SS, AX
    RET
tss_load:
    MOV AX, 0x28 ; 0x28 because it's (index of the TSS entry) << 3
    LTR AX
    RET