#include <stdint.h>
#include <stddef.h>
#pragma once

typedef struct
{
    size_t row;
    size_t column;
    uint8_t color;
} Terminal;

Terminal InitTerminal();
void Term_SetColor(Terminal *term, uint8_t color);
void Term_PrintString(Terminal *term, char *data);