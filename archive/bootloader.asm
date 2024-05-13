[BITS 16]
org 0x7C00

mov ah, 0x00
mov al, 0x03
int 10h

enter_protected_mode:
    cli ; disable interrupts

    ; Fast A20 Enable - supported on most modern computers
    in al, 0x92
    or al, 2
    out 0x92, al

    ; ds is uninitialized, lgdt instruction uses ds as segment so we should init it
    xor ax, ax ; set ax to 0
    mov ds, ax

    ; load gdt
    lgdt [GDT_PTR]

    mov eax, 0x11 ; paging disabled, protection bit eneable. bit4, extension type is always 1
    ; TODO: paging might need to be re-enabled
    mov cr0, eax

    ; far jump
    jmp GDT_BOOT_CS-GDT:start_32 ; GDT_BOOT_CS - GDT = offset into GDT of code segment

align 4
; 6 byte struct
GDT_PTR:
dw GDT_END-GDT-1 ; size-1
dd GDT ; offset

align 16
GDT:
; NULL entry
GDT_NULL: dq 0
; base = 0x00000000
; limit = 0xFFFFF = full 4GB
; flags = 0xC = 0b1100 (4 KB granularity, 32bit)
; access = 0x92 = 10010010 (present, ring 0, code/data segment, writeable)
GDT_BOOT_DS: dq 0x00CF92000000FFFFF
GDT_BOOT_CS: dq 0x00CF9A000000FFFFF ; same as ds but with exec set in access byte

GDT_END:

[BITS 32]
start_32:
    ; we're now in protected mode!!!!
    hlt
    jmp start_32

times 510-($-$$) db 0
dw 0xAA55