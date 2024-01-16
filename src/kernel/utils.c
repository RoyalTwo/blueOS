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

void memory_copy(char *source, char *destination, int nbytes)
{
    for (int i = 0; i < nbytes; i++)
    {
        *(destination + i) = *(source + i);
    }
}