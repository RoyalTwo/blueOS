#pragma once
#include <stdint.h>
#include <stddef.h>

typedef struct
{
    size_t row;
    size_t column;
    uint8_t color;
} Terminal;

Terminal *InitTerminal();
void Term_SetColor(uint8_t color);
void Term_PrintString(char *data);