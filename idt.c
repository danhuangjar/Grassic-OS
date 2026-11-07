// idt.c
#include <stdint.h>
extern void term_print(const char* str);
extern void term_putc(char c); // 加上这一行

// ================== 基础端口读写 ==================
void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}
uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ( "inb %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}
// ================== IDT 结构 ==================
struct idt_entry {
    uint16_t base_low;
    uint16_t sel;
    uint8_t always0;
    uint8_t flags;
    uint16_t base_high;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

struct idt_entry idt[256];
struct idt_ptr idtp;

// 声明外部函数
extern void idt_load(uint32_t);
extern void keyboard_handler();
extern void term_print(const char* str);

// ================== 提前声明 do_keyboard ==================
void do_keyboard(); 

// 给汇编用的中间人
__asm__(".global _do_keyboard_wrapper");
void _do_keyboard_wrapper() __attribute__((used));
void _do_keyboard_wrapper() {
    do_keyboard();
}

// ================== IDT 操作 ==================
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_low = base & 0xFFFF;
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].sel = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags;
}

void init_pic() {
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    
    // 只开启键盘中断 (IRQ 1)，屏蔽其他
    outb(0x21, 0xFD); 
    outb(0xA1, 0xFF);
}

void init_idt() {
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base = (uint32_t)&idt;

    for(int i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0, 0);
    }

    idt_set_gate(33, (uint32_t)keyboard_handler, 0x08, 0x8E);
    
    idt_load((uint32_t)&idtp);
    init_pic();
}

// ================== 键盘映射与处理 ==================
unsigned char kbd_map[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0,
  '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0, '*',   0, ' ', 0
};

void do_keyboard() {
    uint8_t scancode = inb(0x60);
    if (scancode < 128) {
        char c = kbd_map[scancode];
        
        // 特殊处理退格键 (扫描码为 14)
        if (scancode == 14) {
            term_putc('\b');
        } 
        // 特殊处理回车键 (扫描码为 28)
        else if (scancode == 28) {
            term_print("\n> "); // 换行，并重新打印提示符
        }
        // 普通字符
        else if (c != 0) {
            char str[2] = {c, '\0'};
            term_print(str);
        }
    }
}
    // 注意：EOI 信号我们在 isr.asm 里发了，这里不用再发，防止卡死
