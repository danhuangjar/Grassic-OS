; boot.asm
bits 32

section .text
    ; 多字节引导规范 (Multiboot)
    align 4
    dd 0x1BADB002
    dd 0x00
    dd - (0x1BADB002 + 0x00)

global start
extern kernel_main ; 声明外部C语言函数

start:
    cli            ; 关闭中断
    mov esp, stack_space ; 设置栈指针
    call kernel_main     ; 调用C语言写的内核主函数
    hlt            ; 停机

section .bss
resb 8192          ; 预留8KB作为栈空间
stack_space:
section .note.GNU-stack noalloc noexec nowrite progbits