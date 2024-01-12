#include <stdint.h>
#include "vga.h"
#include "terminal.h"

void main()
{
    uint8_t term_color = get_vga_color(VGA_COLOR_LIGHTRED, VGA_COLOR_DARKGRAY);
    Terminal terminal = InitTerminal();
    Term_PrintString(&terminal, "True!\nThis is new");
}