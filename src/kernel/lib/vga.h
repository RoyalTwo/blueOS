#pragma once
#include <stdint.h>
#define VID_MEM_START 0xb8000

typedef enum VGA_COLOR
{
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHTGRAY = 7,
    VGA_COLOR_DARKGRAY = 8,
    VGA_COLOR_LIGHTBLUE = 9,
    VGA_COLOR_LIGHTGREEN = 10,
    VGA_COLOR_LIGHTCYAN = 11,
    VGA_COLOR_LIGHTRED = 12,
    VGA_COLOR_PINK = 13,
    VGA_COLOR_YELLOW = 14,
    VGA_COLOR_WHITE = 15,
} VGA_COLOR;

uint8_t get_vga_color(VGA_COLOR foreground, VGA_COLOR background);
void vga_clear_screen(uint8_t color);
int vga_print(char *input, uint8_t color, int pos);