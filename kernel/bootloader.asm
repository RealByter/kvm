[bits 16]
global start
start:
    ; Set up segment registers
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00

    ; Enable A20 line
    ; in al, 0x92
    ; or al, 2
    ; out 0x92, al

    ; Load GDT
    lgdt [gdt_descriptor]

    ; Switch to protected mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; Far jump to 32-bit code
    jmp 0x08:protected_mode

[bits 32]
protected_mode:
    ; Set up segment registers for 32-bit mode
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Set up stack
    mov esp, 0x7c00

    ; Jump to C code
    extern main
    call main
    
    ; Halt the CPU
    cli
    hlt

; GDT
gdt_start:
    dd 0, 0 ; Null descriptor
gdt_code:
    dw 0xffff    ; Limit
    dw 0         ; Base (low)
    db 0         ; Base (middle)
    db 10011010b ; Access
    db 11001111b ; Granularity
    db 0         ; Base (high)
gdt_data:
    dw 0xffff
    dw 0
    db 0
    db 10010010b
    db 11001111b
    db 0
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

times 510-($-$$) db 0
dw 0xaa55