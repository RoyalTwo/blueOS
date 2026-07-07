#pragma once

typedef enum
{
    KERNEL_LVL = 0,
    USER_LVL = 3,
} CODE_PRIVILEGE_LEVEL;

#define KERNEL_CODE_SELECTOR 1 << 3
#define KERNEL_DATA_SELECTOR 2 << 3
#define USER_DATA_SELECTOR 3 << 3
#define USER_CODE_SELECTOR 4 << 3