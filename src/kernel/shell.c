#include "shell.h"
#include "terminal.h"
#include "vga.h"
#include <stdint.h>

void InitShell()
{
    Term_PrintString("\nShell loaded!");
    Term_PrintString("\n");
    sh_newline();
}

void ShellHandleCommands()
{
}

void sh_newline()
{
    uint8_t new_color = get_vga_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    Term_SetColor(new_color);
    Term_PrintString("\nOS> ");
    uint8_t old_color = get_vga_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    Term_SetColor(old_color);
}