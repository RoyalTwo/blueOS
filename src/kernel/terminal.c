#include "terminal.h"
#include "vga.h"
#include "utils.h"
#include "stdio.h"
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

int get_offset(size_t column, size_t row)
{
    return (SCREEN_COLS * row) + column;
}

void term_scroll()
{
    for (int i = 0; i < SCREEN_ROWS; i++)
    {
        char *below_row = (char *)(VID_MEM_START + (2 * (get_offset(0, i + 1))));
        char *above_row = (char *)(VID_MEM_START + (2 * (get_offset(0, i))));
        memory_copy(below_row, above_row, SCREEN_COLS * 2);
    }
    char *last_row = (char *)(VID_MEM_START + (2 * (get_offset(0, SCREEN_ROWS - 1))));
    for (int i = 0; i < SCREEN_COLS; i++)
        last_row[i] = 0;
}
void Term_SetColor(uint8_t color)
{
    terminal.color = color;
}

void term_put_char(char c)
{
    if ((int)c == 10)
    {
        // New line character
        terminal.column = 0;
        terminal.row++;
    }
    else
    {
        vga_print_at(c, terminal.color, (terminal.row * SCREEN_COLS) + terminal.column);
        terminal.column++;
    }

    // Handle going to new line
    if (terminal.column == SCREEN_COLS)
    {
        terminal.column = 0;
        terminal.row++;
    }
    if (terminal.row == SCREEN_ROWS)
    {
        term_scroll();
        terminal.row--;
    }
    // TODO: Shouldn't set cursor pos on every character
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