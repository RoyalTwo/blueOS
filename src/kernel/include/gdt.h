#pragma once

void gdt_init();

typedef enum
{
    KERNEL_LVL = 0,
    USER_LVL = 3,
} CODE_PRIVILEGE_LEVEL;