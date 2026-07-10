#pragma once
#include <limine.h>
#include <stdint.h>

struct kernel_positions
{
    uint64_t physical_base;
    uint64_t virtual_base;
};

typedef struct
{
    struct limine_framebuffer *framebuffer;
    struct limine_memmap_response *memmap;
    uint64_t hhdm_offset;
    struct kernel_positions kernel_pos;
} kernel_t;

extern kernel_t kernel;