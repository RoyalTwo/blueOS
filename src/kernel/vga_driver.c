#include "vga.h"
#define SCREEN_ROWS 25
#define SCREEN_COLS 80

uint8_t get_vga_color(VGA_COLOR foreground, VGA_COLOR background)
{
    // Little-endian, background is lower half
    return foreground | background << 4;
}

void vga_clear_screen(char *display_mem, int color)
{
    int area = SCREEN_ROWS * SCREEN_COLS;
    for (int pos = 0; pos < area * 2; pos++)
    {
        // Video memory is two parts, character and color/attribute
        display_mem[pos] = (uint8_t)' ';
        display_mem[++pos] = color;
    }
}