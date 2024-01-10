;
;
;   This file is copied from an early version of the bootloader. It only serves to have something to load, and will be replaced immediately.
;
;









org 0x7C00
bits 16

start:
    jmp main

puts:
    push si             ; Save si
    push ax             ; Save ax since you're modifying al and ah
.puts_loop:
    lodsb               ; Loads byte at DS:SI to al and increments si
    or al, al           ; ORing a value with itself only sets Zero Flag register if the value is 0, nothing else
    jz .puts_done            ; jumps to .done if zero flag is set

    mov ah, 0eh         ; Teletype Output
    mov bh, 0           ; Page Number
    int 10h

    jmp .puts_loop
.puts_done:
    pop si
    pop ax
    ret

reset_disk:
    push ax
.reset_loop:
    ; Reset disk system (we want to make sure we start at 0, int sets it to 0)
    mov ah, 0           ; Set function to "reset"
    mov dl, 0           ; Our disk is drive 0
    int 0x13            ; Sets carry flag if failed
    jc .reset_loop      ; Retry if failed
    jmp .reset_done
.reset_done:
    pop ax
    ret

main:
    ; Initialize data segments
    mov ax, 0           ; Can't write to ds and es directly
    mov ds, ax
    mov es, ax

    ; Initialize stack
    mov ss, ax          ; Set to 0 to start
    mov sp, 0x7C00      ; Stack grows down, don't want to overwrite bootloader

    ; Print message
    mov si, message
    call puts

    ; Reading other segments
    call reset_disk

    hlt

; message is Data, so DS
message: db "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG", 0

times 510-($-$$) db 0

dw 0AA55h