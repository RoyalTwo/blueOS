#include "terminal.h"
#include "vga.h"
#include "utils.h"
#define SCREEN_ROWS 25
#define SCREEN_COLS 80

// Caps indicate public function
Terminal InitTerminal()
{
    uint8_t color = get_vga_color(VGA_COLOR_WHITE, VGA_COLOR_CYAN);
    Terminal term = {0,
                     0,
                     color};
    vga_clear_screen(term.color);

    return term;
}

void term_reset_pos(Terminal *term)
{
    term->column = 0;
    term->row = 0;
}

void Term_SetColor(Terminal *term, uint8_t color)
{
    term->color = color;
    vga_clear_screen(term->color);
    term_reset_pos(term);
}

void term_put_char_at(Terminal *term, char c, size_t x, size_t y)
{
    char arr[] = {c};
    vga_print_at(arr, term->color, (y * SCREEN_COLS) + x);
    // Should probably update Terminal later, but how? Should position then be after inserted char?
}

void term_put_char(Terminal *term, char c)
{
    if ((int)c == 10)
    {
        // New line character
        term->column = 0;
        if (term->row + 1 == SCREEN_ROWS)
            term->row = 0;
        else
            term->row = term->row + 1;
        return;
    }
    char arr[] = {c};
    vga_print_at(arr, term->color, (term->row * SCREEN_COLS) + term->column);
    if (term->column + 1 == SCREEN_COLS)
    {
        term->column = 0;
        if (term->row + 1 == SCREEN_ROWS)
        {
            term->row = 0;
        }
        else
        {
            term->row++;
        }
    }
    else
    {
        term->column++;
    }
}

void term_write(Terminal *term, char *data, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        term_put_char(term, data[i]);
    }
}

void Term_PrintString(Terminal *term, char *data)
{
    term_write(term, data, strlen(data));
}