#include <stdint.h>
#include "vga.h"
#define VID_MEM_START 0xb8000

void main()
{
    char *display_mem = (char *)VID_MEM_START;
    uint8_t term_color = get_vga_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_clear_screen(display_mem, term_color);
    uint8_t color = get_vga_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_print(display_mem, "Welcome to gladOS!", color, (11 * 80 + (40 - 9)));
    vga_print(display_mem, "hello :D", color, 0);
}