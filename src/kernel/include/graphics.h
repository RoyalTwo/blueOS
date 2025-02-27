#pragma once
#include <stdint.h>

// config
void gpu_init(struct limine_framebuffer *fb);
void term_setcolor(uint32_t foreground, uint32_t background);

// drawing/writing
void putc(char c);
void draw_square(int x, int y, int length, uint32_t color);
void put_pixel(int x, int y);
void puts(const char *str);