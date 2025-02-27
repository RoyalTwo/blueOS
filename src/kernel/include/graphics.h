#pragma once
#include <stdint.h>

// config
void gpu_init(struct limine_framebuffer *fb);
void term_setcolor(uint32_t foreground, uint32_t background);

// drawing/writing
// TODO: Writing should be moved to libk eventually
void putc(char c);
void draw_square(int x, int y, int length, uint32_t color);
void put_pixel(int x, int y);
void print_string(const char *str);
void kprintf(char *fmt, ...);