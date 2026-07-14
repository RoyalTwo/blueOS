#pragma once

#define CLR "\033[2J \033[H"

void init_serial();
void write_serial(const char *str);
void write_serial_char(char ch);
