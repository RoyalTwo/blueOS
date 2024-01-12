#include <stdint.h>
// stdint.h is included with the compiler
#define VID_MEM_START 0xb8000
#define SCREEN_ROWS 25
#define SCREEN_COLS 80

void test_main();

// Returns number of characters printed
int kprint(char *display_mem, char *input, int pos)
{
    if (pos < 0)
        return 0;
    // if pos is not even, that's the color part of memory
    if (pos % 2 != 0)
        pos = pos - 1;

    int str_pos = 0;
    char current = input[str_pos];
    while (current != '\0')
    {
        display_mem[pos] = current;
        pos = pos + 2;
        str_pos++;
        current = input[str_pos];
    }
    return str_pos;
}

void kclear_screen(char *display_mem, char clear_to, int color)
{
    int area = SCREEN_ROWS * SCREEN_COLS;
    for (int pos = 0; pos < area * 2; pos++)
    {
        // Video memory is two parts, character and color/attribute
        display_mem[pos] = clear_to;
        display_mem[++pos] = color;
    }
}

void main()
{
    char *display_mem = (char *)VID_MEM_START;
    kclear_screen(display_mem, '\0', 15);
    kprint(display_mem, "Welcome to gladOS!", 0);
}