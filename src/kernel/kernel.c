#include <stdint.h>
#include "vga.h"

void main()
{
    uint8_t term_color = get_vga_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_clear_screen(term_color);
    uint8_t color = get_vga_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_print("Welcome to gladOS!", color, (11 * 80 + (40 - 9)));
    vga_print("hello :D", color, 0);
}