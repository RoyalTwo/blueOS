#pragma once
#include <stdint.h>
#include <limine.h>

// config
void tty_init(struct limine_framebuffer *fb);
void term_setcolor(uint32_t foreground, uint32_t background);

// drawing/writing
// TODO: Writing should be moved to libk eventually
void putc(char c);
void print_string(const char *str);
void kprintf(char *fmt, ...);