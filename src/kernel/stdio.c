#include "stdio.h"
#include "terminal.h"
#include <stdint.h>

// Defined in terminal.c
void term_put_char(char c);

int *printf_num(int *argp, int length, int radix);

#define PRINTF_STATE_NORMAL 0
#define PRINTF_STATE_SPEC 1

#define PRINTF_LENGTH_DEFAULT 0
// Specifiers: %c, %s, %d, %x
void __attribute__((cdecl)) printf(const char *fmt, ...)
{
    int *argp = (int *)(int *)&fmt;
    // IMPORTANT: This only handles near pointers. Should divide sizeof(fmt) by sizeof(int) to handle far pointers
    argp++; // Point to next argument after fmt
    int state = PRINTF_STATE_NORMAL;
    // Length is for future implementation of length specifier
    int length = PRINTF_LENGTH_DEFAULT;
    int radix = 10; // Base 10
    // TODO: Implement unsigned ints, length, etc

    while (*fmt)
    {
        switch (state)
        {
        case PRINTF_STATE_NORMAL:
            // Decide what to do based on current character
            switch (*fmt)
            {
            case '%':
                // if char is %, go to length_state with next char
                state = PRINTF_STATE_SPEC;
                break;
                // fmt is incremented at the end already
            // if char is regular, put_char
            default:
                term_put_char(*fmt);
                break;
            }

            break;
        case PRINTF_STATE_SPEC:
            // Current char is either s, c, x, or d (for now)
            switch (*fmt)
            {
            case 'c':
                term_put_char((char)*argp);
                argp++;
                break;
            case 's':
                // if char is s, print string at argp and increment argp
                Term_PrintString(*(char **)argp);
                argp++;
                break;
            case '%':
                term_put_char('%');
                break;
            case 'd':
                radix = 10;
                argp = printf_num(argp, length, radix); // Takes argp, length, base, and returns the pointer to the next argument
                break;
            case 'x':
                radix = 16;
                argp = printf_num(argp, length, radix);
                break;

            default:
                // Ignore invalid specifiers
                break;
            }
            // Reset to normal state after processing specifier
            state = PRINTF_STATE_NORMAL;
            length = PRINTF_LENGTH_DEFAULT;
            radix = 10;
        }
        // Increment fmt to be each character
        fmt++;
    }
}

const char HexChars[] = "0123456789abcdef";
int *printf_num(int *argp, int length, int radix)
{
    char buffer[32];
    unsigned long long number; // convert to hold everything
    int number_sign = 1;
    int pos = 0;

    switch (length)
    {
    // This is setup for length specifiers
    // Right now we only have default length which is int or lower
    case PRINTF_LENGTH_DEFAULT:
        int n = *argp;
        // number is unsigned, so we have to convert this to unsigned
        if (n < 0)
        {
            n = -n;
            number_sign = -1;
        }
        number = n;
        argp++; // argp can be incremented by an int since we don't use larger than int specifiers yet
    }

    // Convert number to ASCII
    do
    {
        // Divide number by base to get remainder
        uint32_t remainder = number % radix;
        number = number / radix;
        buffer[pos++] = HexChars[remainder];

    } while (number > 0);
    if (number_sign == -1)
    {
        buffer[pos++] = '-';
    }

    while (--pos >= 0)
    {
        term_put_char(buffer[pos]);
    }
    return argp;
}