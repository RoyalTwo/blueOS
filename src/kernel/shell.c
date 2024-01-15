#include "shell.h"
#include "terminal.h"
#include "stdio.h"
#include "vga.h"
#include <stdint.h>

void InitShell()
{
    printf("\n");
    printf("Shell loaded!");
    sh_newline();
}

void ShellHandleCommands()
{
}

void sh_newline()
{
    uint8_t new_color = get_vga_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    Term_SetColor(new_color);
    printf("\nOS> ");
    uint8_t old_color = get_vga_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    Term_SetColor(old_color);
}