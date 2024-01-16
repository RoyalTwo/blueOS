#include "terminal.h"
#include "vga.h"
#include "utils.h"
#define SCREEN_ROWS 25
#define SCREEN_COLS 80

static char *VID_MEM = (char *)VID_MEM_START;

Terminal terminal;

// Caps indicate public function
Terminal *InitTerminal()
{
    uint8_t color = get_vga_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    terminal.column = 0;
    terminal.row = 0;
    terminal.color = color;
    vga_clear_screen(terminal.color);

    return &terminal;
}

void term_reset_pos()
{
    terminal.column = 0;
    terminal.row = 0;
    set_cursor_pos(0);
}

void Term_SetColor(uint8_t color)
{
    terminal.color = color;
}

void term_put_char_at(char c, size_t x, size_t y)
{
    vga_print_at(c, terminal.color, (y * SCREEN_COLS) + x);
    // Should probably update Terminal later, but how? Should position then be after inserted char?
}

void term_put_char(char c)
{
    if ((int)c == 10)
    {
        // New line character
        terminal.column = 0;
        if (terminal.row + 1 == SCREEN_ROWS)
            terminal.row = 0;
        else
            terminal.row = terminal.row + 1;
        set_cursor_pos((terminal.row - 1) * SCREEN_COLS);
        return;
    }
    vga_print_at(c, terminal.color, (terminal.row * SCREEN_COLS) + terminal.column);

    // Cursor position
    if (terminal.column + 1 == SCREEN_COLS)
    {
        terminal.column = 0;
        if (terminal.row + 1 == SCREEN_ROWS)
        {
            terminal.row = 0;
        }
        else
        {
            terminal.row++;
        }
    }
    else
    {
        terminal.column++;
    }
    set_cursor_pos((terminal.row * SCREEN_COLS) + terminal.column);
}

void term_write(char *data, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        term_put_char(data[i]);
    }
}

void Term_PrintString(char *data)
{
    term_write(data, strlen(data));
}