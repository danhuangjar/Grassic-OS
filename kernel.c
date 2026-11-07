// kernel.c
#include <stdint.h> // 必须加上这个，用来识别 uint16_t 等类型

volatile unsigned short* vga_buffer = (unsigned short*)0xB8000;
const int VGA_COLS = 80;
const int VGA_ROWS = 25;

int term_col = 0;
int term_row = 0;
unsigned char term_color = 0x0A; 

// 声明外部的端口输出函数（在 idt.c 里实现的）
extern void outb(uint16_t port, uint8_t val);

void update_hardware_cursor() {
    uint16_t pos = term_row * VGA_COLS + term_col;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void term_clear() {
    for (int i = 0; i < VGA_COLS * VGA_ROWS; i++) {
        vga_buffer[i] = (term_color << 8) | ' ';
    }
    term_col = 0;
    term_row = 0;
    update_hardware_cursor();
}

void term_putc(char c) {
    if (c == '\n') {
        term_col = 0;
        term_row++;
    } else if (c == '\b') { 
        if (term_col > 2) { // 防止把 "> " 删掉
            term_col--;
            vga_buffer[term_row * VGA_COLS + term_col] = (term_color << 8) | ' ';
        }
    } else {
        vga_buffer[term_row * VGA_COLS + term_col] = (term_color << 8) | c;
        term_col++;
    }

    // 处理自动换行
    if (term_col >= VGA_COLS) {
        term_col = 0;
        term_row++;
    }

    // 处理屏幕滚动
    if (term_row >= VGA_ROWS) {
        for (int i = 0; i < (VGA_ROWS - 1) * VGA_COLS; i++) {
            vga_buffer[i] = vga_buffer[i + VGA_COLS];
        }
        for (int i = (VGA_ROWS - 1) * VGA_COLS; i < VGA_ROWS * VGA_COLS; i++) {
            vga_buffer[i] = (term_color << 8) | ' ';
        }
        term_row = VGA_ROWS - 1;
    }
    update_hardware_cursor();
}

void term_print(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        term_putc(str[i]);
    }
}

extern void init_idt();
extern void init_gdt();

void grassic_init() {
    term_clear();
    term_print("========================================\n");
    term_print("        GRASSIC OS v0.4 (Terminal)      \n");
    term_print("========================================\n\n");
    term_print("Type something on your keyboard:\n\n> ");
}

void kernel_main() {
    grassic_init();
    init_gdt();
    init_idt();
    __asm__ volatile ("sti");
    while (1) {
        __asm__ volatile ("hlt"); 
    } 
}