#include <stdint.h>
#include "vga.h"
#include "terminal.h"

void main()
{
    Terminal *term_handle = InitTerminal();
    Term_PrintString("Kernel loaded!");
}