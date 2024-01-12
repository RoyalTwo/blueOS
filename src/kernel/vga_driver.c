#include "vga.h"
#define SCREEN_ROWS 25
#define SCREEN_COLS 80

char *VID_MEM = (char *)VID_MEM_START;

uint8_t get_vga_color(VGA_COLOR foreground, VGA_COLOR background)
{
    // Little-endian, background is lower half
    return foreground | background << 4;
}

void vga_clear_screen(uint8_t color)
{
    int area = SCREEN_ROWS * SCREEN_COLS;
    for (int pos = 0; pos < area * 2; pos++)
    {
        // Video memory is two parts, character and color/attribute
        VID_MEM[pos] = (uint8_t)' ';
        VID_MEM[++pos] = color;
    }
}

// Returns number of characters printed
int vga_print(char *input, uint8_t color, int pos)
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
        VID_MEM[pos] = current;
        pos++;
        VID_MEM[pos] = color;
        pos++;
        str_pos++;
        current = input[str_pos];
    }
    return str_pos;
}