org 0x7C00
bits 16

%define ENDL 0x0D, 0x0A
;
;   FAT12 Headers (REQUIRED)
;

jmp short start
nop
bdb_oem:                    db 'MSWIN4.1'       ; 8 bytes
bdb_bytes_per_sector:       dw 512              ; 2 bytes
bdb_sectors_per_cluster:    db 18               ; 1 byte
bdb_reserved_sectors:       dw 1                ; 2 bytes
bdb_fat_count:              db 2                ; 1 bytes
bdb_dir_entries_count:      dw 0E0h             ; 2 bytes
bdb_total_sectors:          dw 2880             ; 2 bytes, 2880 * 512 = 1.44 MB
bdb_media_descriptor_type:  db 0F0h             ; F0 = 3.5" Floppy Disk
bdb_sectors_per_fat:        dw 9                ; 9 sectors/FAT
bdb_sectors_per_track:      dw 18               ; 2 bytes
bdb_heads:                  dw 2                ; 2 bytes
bdb_hidden_sectors:         dd 0                ; 4 bytes
bdb_large_sectors:          dd 0                ; 4 bytes

; Extended Boot Record
ebr_drive_number:           db 0                ; 0x00 = floppy disk
                            db 0                ; reserved
ebr_signature:              db 29h              ; either 28h or 29h
ebr_serial_num:             db 12h, 34h, 56h, 78h ; Doesn't matter
ebr_volume_label:           db 'LOS        '    ; 11 bytes padded with spaces
ebr_system_id:              db 'FAT12   '       ; 8 bytes padded with spaces


;
;   Code
;

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

;
; Resets drive
; Input: DL = Drive number
;
reset_disk:
    push ax
.reset_loop:
    ; Reset disk system (we want to make sure we start at 0, int sets it to 0)
    mov ah, 0           ; Set function to "reset"
    ; DL is set by caller
    int 0x13            ; Sets carry flag if failed
    jc .reset_loop      ; Retry if failed
    ; TODO: This could get stuck in an infinite loop technically.
    jmp .reset_done
.reset_done:
    pop ax
    ret

;
;   Converts LBA address to CHS address
;   Input: AX = LBA
;   Output: CH = Cylinder, CL = Sector, DH = Head
;
lba_to_chs:
    ; Formulas:
    ; HPC = Max Heads per Cylinder = bdb_heads
    ; SPT = Max Sectors per Track = bdb_sectors_per_track
    ; C = (LBA / SPT) / HPC
    ; H = (LBA / SPT) % HPC
    ; S = (LBA % SPT) + 1

    ; DL and AX are not part of output but will be modified, so let's save them for the caller
    push ax
    ; Can't push 8 bit registers to stack, have to save whole thing
    push dx

    xor dx, dx                          ; Set dx to 0
    ; div divides DX:AX by value passed in, stores remainder in dx, result in ax
    div word [bdb_sectors_per_track]    ; ax = LBA / SPT, dx = LBA % SPT
    inc dx                              ; dx = (LBA % SPT) + 1 = Sector
    mov cx, dx                          ; Store value of dx since dx will change

    xor dx, dx                          ; Reset dx to 0
    div word [bdb_heads]                ; ax = (LBA / SPT) / HPC, dx = (LBA / SPT) % HPC
                                        ; Now just need to move values around
                                        ; At this point, ax = Cylinder, dx = Head, cx = Sector
                                        ; We need: CH = Cylinder, DH = Head, CL = Sector
                                        ; Little-endian, so l is the most significant digits
    mov dh, dl                          ; Move Head to DH
    mov ch, al                          ; This is first 8 bits, we need 10 bits
    shl ah, 6                           ; If we had ax = al|1111 1111 ah|0000 0011, ah now equals 1100 0000
                                        ; OR changes each bit to be 0 if x and y are 0 at that bit, or 1 otherwise.
    or cl, ah                           ; This combines them. If ah = 1100 0000 and cl = 0000 0011, OR would make that 1100 0011

    ; Restore AX and DL
    pop ax                              ; Pops dx into ax since stack is LIFO
    mov dl, al
    pop ax                              ; Now pops ax into ax
    ret

;
;   Reads sectors from disk into RAM
;   Input:
;       - AX = LBA address to start
;       - CL = Number of sectors to read
;       - DL = Drive number
;       - ES:BX = Memory location to read to
;
read_disk:
    ; Save registers that will be modified
    push ax
    push bx
    push cx
    push dx
    push di

    push cx                             ; lba_to_chs overwrites CX, we need to save it
    call lba_to_chs                     ; CH = Cylinder, CL = Sector, DH = Head

    pop ax                              ; ax = previous cx, so al now = number of sectors to read
    mov ah, 02h
                                        ; CH, CL, DH are already set for us
                                        ; DL already equals the drive number
                                        ; ES:BX is passed by caller

    ; We should retry reading multiple times
    mov di, 3                           ; Num of times to retry
.retry:
    ; We don't know what INT 13h overwrites, so let's save all registers first
    pusha
    stc                                 ; Incase BIOS didn't set it yet
    int 13h
    jnc .success                        ; int 13h sets CF to 0 if successful, 1 if failure

    ; If here, read failed
    popa
    call reset_disk
    dec di
    test di, di                         ; Sets ZF when di is 0. Performs an AND, if di > 0, di AND di != 0. If di == 0, 0 AND 0 == 0
    jnz .retry
.fail:
    ; If here, di was 0 so did not jump, therefore we've tried 3 times and it still failed.
    mov si, read_failure_msg
    call puts
    jmp $
    hlt
.success:
    popa

    mov si, read_success_msg
    call puts
    
    ; Restore registers we modified
    pop di
    pop dx
    pop cx
    pop bx
    pop ax
    ret
    
    ; INT 13h
    ; Requires:
    ; AH = 02h | Function
    ; AL | Num of Sectors to Read
    ; CH | Cylinder Number to read from
    ; CL | Sector Number to read from
    ; DH | Head Number to read from
    ; DL | Drive Number to read from
    ; ES:BX | Address of where to read sectors to in RAM
    ; Returns:
    ; CF | Set on error
    ; AH | Return code
    ; AL | Num of sectors read

main:
    ; Initialize data segments
    mov ax, 0           ; Can't write to ds and es directly
    mov ds, ax
    mov es, ax

    ; Initialize stack
    mov ss, ax          ; Set to 0 to start
    mov sp, 0x7C00      ; Stack grows down, don't want to overwrite bootloader

    ; Reading other segments
    ; Set variables
    ; BIOS sets disk number we're on into dl by default
    mov [ebr_drive_number], dl      ; Let's save it just in case
    call reset_disk
;       - AX = LBA address to start
;       - CL = Number of sectors to read
;       - DL = Drive number
;       - ES:BX = Memory location to read to
    mov ax, 1                       ; LBA=1, second sector from disk (we're in first already)
    mov cl, 1                       ; Just one sector for now
    mov bx, 0x7E00                  ; 512 bytes after start of bootloader, which is 512 bytes
    call read_disk

    hlt

read_success_msg: db "Disk read successful!", ENDL, 0
read_failure_msg: db "Disk read failed! ", ENDL, 0

times 510-($-$$) db 0

dw 0AA55h