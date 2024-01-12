#include "vga.h"
#include "utils.h"
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

void vga_print_at(char *input, uint8_t color, int pos)
{
    if (pos < 0)
        return;
    pos = pos * 2;

    size_t input_length = strlen(input);
    for (int str_pos = 0; str_pos < input_length; str_pos++)
    {
        VID_MEM[pos] = input[str_pos];
        pos++;
        VID_MEM[pos] = color;
        pos++;
    }
}