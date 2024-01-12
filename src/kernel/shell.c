#include "shell.h"
#include "terminal.h"
#include "vga.h"
#include <stdint.h>

void InitShell()
{
    Term_PrintString("\n");
    sh_newline();
}

void ShellHandleCommands()
{
}

void sh_newline()
{
    uint8_t new_color = get_vga_color(VGA_COLOR_CYAN, VGA_COLOR_BLACK);
    Term_PrintString("\nOS>");
}