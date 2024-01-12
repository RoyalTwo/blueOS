#include <stdint.h>
#include "vga.h"
#define VID_MEM_START 0xb8000

// Returns number of characters printed
int kprint(char *display_mem, char *input, uint8_t color, int pos)
{
    if (pos < 0)
        return 0;
    // if pos is not even, that's the color part of memory
    if (pos % 2 != 0)
        pos = pos - 1;
    pos = pos * 2;

    int str_pos = 0;
    char current = input[str_pos];
    while (current != '\0')
    {
        display_mem[pos] = current;
        pos++;
        display_mem[pos] = color;
        pos++;
        str_pos++;
        current = input[str_pos];
    }
    return str_pos;
}

void main()
{
    char *display_mem = (char *)VID_MEM_START;
    uint8_t term_color = get_vga_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_clear_screen(display_mem, term_color);
    uint8_t color = get_vga_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    kprint(display_mem, "Welcome to gladOS!", color, (11 * 80 + (40 - 9)));
    kprint(display_mem, "hello :D", color, 0);
}