#include "utils.h"

size_t strlen(char *data)
{
    // We're passed "hello\0"
    size_t length = 0;
    while (data[length])
    {
        length++;
    }
    return length;
}