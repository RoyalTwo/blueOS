#pragma once
#include <stdint.h>

void gpu_init(struct limine_framebuffer *fb);
void putc(char c);
void draw_square(int x, int y, int length, uint32_t color);
void put_pixel(int x, int y);
void puts(const char *str);