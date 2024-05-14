#include "terminal.h"

void kernel_main(void)
{
    terminal_initialize();
    terminal_writestring("Terminal Initialized\n");
    terminal_writestring("Kernel Loaded\n");

    // Kernel should never exit
    while (1)
    {
        asm("hlt");
    }
}