#pragma once
#include <limine.h>
#include <stdint.h>

struct kernel_positions
{
    uint64_t physical_base;
    uint64_t virtual_base;
};

// ONLY INITIALIZERS SHOULD MODIFY THESE VALUES
typedef struct
{
    struct limine_framebuffer *framebuffer;
    struct limine_memmap_response *memmap;
    uint64_t hhdm_offset;
    struct kernel_positions kernel_pos;
    struct
    {
        uint8_t *bitmap;
    } pmm;
} kernel_t;

extern kernel_t kernel;