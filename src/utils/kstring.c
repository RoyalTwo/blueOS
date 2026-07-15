#include <stdint.h>
#include <kernel/kstring.h>

void *memcpy(void *dest, const void *src, size_t n)
{
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    for (size_t i = 0; i < n; i++)
    {
        pdest[i] = psrc[i];
    }

    return dest;
}
void *memset(void *s, int c, size_t n)
{
    uint8_t *p = (uint8_t *)s;

    for (size_t i = 0; i < n; i++)
    {
        p[i] = (uint8_t)c;
    }

    return s;
}
void *memmove(void *dest, const void *src, size_t n)
{
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    if (src > dest)
    {
        for (size_t i = 0; i < n; i++)
        {
            pdest[i] = psrc[i];
        }
    }
    else if (src < dest)
    {
        for (size_t i = n; i > 0; i--)
        {
            pdest[i - 1] = psrc[i - 1];
        }
    }

    return dest;
}
int memcmp(const void *s1, const void *s2, size_t n)
{
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;

    for (size_t i = 0; i < n; i++)
    {
        if (p1[i] != p2[i])
        {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
}

size_t strlen(const char *str)
{
    // BSD implementation
    const char *s;
    for (s = str; *s; ++s)
        ;
    return (s - str);
}

int strncmp(const char *left, const char *right, size_t count)
{
    for (size_t i = 0; i < count; i++)
    {
        unsigned char l = (unsigned char)left[i];
        unsigned char r = (unsigned char)right[i];

        if (l != r)
            return (int)l - (int)r;

        if (l == '\0')
            return 0;
    }

    return 0;
}

void reverse(char str[], int length)
{
    int start = 0;
    int end = length - 1;
    while (start < end)
    {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

int get_num_length(uint64_t num)
{
    int length = 0;
    do
    {
        length++;
        num /= 10;
    } while (num > 0);
    return length;
}

void uint64_to_string(uint64_t num, char *str)
{
    int length = get_num_length(num);
    str[length] = '\0';
    int index = length - 1;
    do
    {
        str[index--] = '0' + (num % 10);
        num /= 10;
    } while (num > 0);
}

void uint64_to_hex_string(uint64_t num, char *str)
{
    char buffer[17];
    int index = 0;
    if (num == 0)
    {
        buffer[index++] = '0';
    }
    else
    {
        while (num > 0)
        {
            uint8_t digit = num & 0xF;
            if (digit < 10)
            {
                buffer[index++] = '0' + digit;
            }
            else
            {
                buffer[index++] = 'A' + (digit - 10);
            }
            num >>= 4;
        }
    }

    buffer[index] = '\0';
    reverse(buffer, index);
    memcpy(str, buffer, 17);
}

void uint64_to_binary_string(uint64_t num, char *buf)
{
    char buffer[65];
    int idx = 0;
    if (num == 0)
    {
        buffer[idx++] = '0';
    }
    else
    {
        while (num > 0)
        {
            buffer[idx++] = (num & 1) ? '1' : '0';
            num >>= 1;
        }
    }
    buffer[idx] = 0;
    reverse(buffer, idx);
    for (int i = 0; i <= idx; i++)
        buf[i] = buffer[i];
}

void uint64_to_hex_string_padded(uint64_t num, char *str)
{
    char buffer[17];
    int index = 0;
    if (num == 0)
    {
        buffer[index++] = '0';
    }
    else
    {
        while (num > 0)
        {
            uint8_t digit = num & 0xF;
            if (digit < 10)
            {
                buffer[index++] = '0' + digit;
            }
            else
            {
                buffer[index++] = 'A' + (digit - 10);
            }
            num >>= 4;
        }
    }
    while (index < 16)
        buffer[index++] = '0';
    buffer[index] = '\0';
    reverse(buffer, index);
    memcpy(str, buffer, 17);
}