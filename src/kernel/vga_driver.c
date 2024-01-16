#include "vga.h"
#include "utils.h"
#include "io.h"
#define SCREEN_ROWS 25
#define SCREEN_COLS 80
#define SCREEN_MEM_CTRL 0x3d4
#define SCREEN_MEM_DATA 0x3d5

static char *VID_MEM = (char *)VID_MEM_START;

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

void vga_print_at(char input, uint8_t color, int pos)
{
    if (pos < 0)
        return;
    pos = pos * 2;

    VID_MEM[pos] = input;
    pos++;
    VID_MEM[pos] = color;
}

int get_cursor_pos()
{
    /* Screen cursor position: ask VGA control register (0x3d4) for bytes
     * 14 = high byte of cursor and 15 = low byte of cursor. */
    port_byte_out(SCREEN_MEM_CTRL, 14); /* Requesting byte 14: high byte of cursor pos */
    /* Data is returned in VGA data register (0x3d5) */
    int position = port_byte_in(SCREEN_MEM_DATA);
    position = position << 8; /* high byte */

    port_byte_out(SCREEN_MEM_CTRL, 15); /* requesting low byte */
    position += port_byte_in(SCREEN_MEM_DATA);
    return position * 2; // Times 2 because 2 is size of each VGA cell (character and attributes)
}
void set_cursor_pos(int offset)
{
    port_byte_out(SCREEN_MEM_CTRL, 14);
    // Set high byte (bitshifted because we previously bitshifted to get cursor pos)
    port_byte_out(SCREEN_MEM_DATA, (unsigned char)(offset >> 8));
    port_byte_out(SCREEN_MEM_CTRL, 15);
    // Set low byte
    port_byte_out(SCREEN_MEM_DATA, (unsigned char)(offset & 0xff));
}