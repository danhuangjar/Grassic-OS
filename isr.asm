; isr.asm
bits 32

global idt_load
global keyboard_handler
global gdt_flush
extern idtp
extern gp

; 告诉 NASM 外部有个叫 _do_keyboard_wrapper 的符号
extern _do_keyboard_wrapper

gdt_flush:
    mov eax, [esp+4]
    lgdt [eax]
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:.flush
.flush:
    ret

idt_load:
    mov eax, [esp+4]
    lidt [eax]
    ret

keyboard_handler:
    pusha
    push ds
    push es
    push fs
    push gs

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; 调用 C 语言的处理函数
    call _do_keyboard_wrapper

    ; 发送 EOI (End of Interrupt) 给 PIC 主片
    mov al, 0x20
    out 0x20, al

    pop gs
    pop fs
    pop es
    pop ds
    popa
    iret

; 告诉链接器不需要可执行栈，消除警告
section .note.GNU-stack noalloc noexec nowrite progbits