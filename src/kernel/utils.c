#include "utils.h"

size_t strlen(char *data)
{
    size_t length = 0;
    while (data[length])
    {
        length++;
    }
    return length;
}