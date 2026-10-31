; ============================================================
; boot.asm - Loads kernel, switches to 32-bit Protected Mode
; ============================================================
[org 0x7C00]
[bits 16]

KERNEL_LOAD_SEGMENT equ 0x0000
KERNEL_LOAD_OFFSET  equ 0x1000
KERNEL_SECTOR_COUNT equ 6          ; enough for the current kernel image
                                      ; building and shrink/grow as needed

start:
    mov [boot_drive], dl
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    mov si, msg_boot
    call print_string

    call load_kernel

    mov si, msg_jump
    call print_string

    call enable_a20
    call switch_to_pm

    cli
    hlt
    jmp $

load_kernel:
    pusha
    mov si, 3
.retry:
    mov ah, 0x00
    mov dl, [boot_drive]
    int 0x13
    jc .retry_dec

    mov ah, 0x02
    mov al, KERNEL_SECTOR_COUNT
    mov ch, 0x00
    mov cl, 0x02
    mov dh, 0x00
    mov dl, [boot_drive]
    mov bx, KERNEL_LOAD_OFFSET
    int 0x13
    jnc .success

.retry_dec:
    dec si
    jnz .retry
    jmp disk_error

.success:
    popa
    ret

boot_drive db 0

disk_error:
    mov si, msg_disk_error
    call print_string
    cli
    hlt
    jmp $

enable_a20:
    in al, 0x92
    or al, 2
    out 0x92, al
    ret

print_string:
    pusha
.next_char:
    lodsb
    cmp al, 0
    je .done
    mov ah, 0x0E
    mov bh, 0x00
    mov bl, 0x07
    int 0x10
    jmp .next_char
.done:
    popa
    ret

gdt_start:
    dq 0x0
gdt_code:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10011010b
    db 11001111b
    db 0x00
gdt_data:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b
    db 11001111b
    db 0x00
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

switch_to_pm:
    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp CODE_SEG:init_pm

[bits 32]
init_pm:
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov ebp, 0x90000
    mov esp, ebp

    jmp KERNEL_LOAD_OFFSET

msg_boot       db 'Bootloader loaded in memory!', 0x0D, 0x0A, 0
msg_jump       db 'Kernel found - entering Protected Mode...', 0x0D, 0x0A, 0
msg_disk_error db 'DISK READ ERROR!', 0x0D, 0x0A, 0

times 510-($-$$) db 0
dw 0xAA55
