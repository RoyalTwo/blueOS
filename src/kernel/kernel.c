#include <stdint.h>
#include "vga.h"
#include "terminal.h"
#include "shell.h"

void main()
{
    InitTerminal();
    Term_PrintString("Kernel loaded!");
    InitShell();
    ShellHandleCommands();
}