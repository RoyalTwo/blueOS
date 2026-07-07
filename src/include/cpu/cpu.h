#pragma once

#define HALT()                           \
    do                                   \
    {                                    \
        __asm__ __volatile__("cli");     \
        for (;;)                         \
        {                                \
            __asm__ __volatile__("hlt"); \
        }                                \
    } while (0)
