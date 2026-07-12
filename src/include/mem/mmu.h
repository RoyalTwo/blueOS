#pragma once

#define PHY_TO_VIRT(physical) (physical + kernel.hhdm_offset) // Requires <kernel/kernel.h> included
#define PAGE_ADDR_MASK 0x000FFFFFFFFFF000ULL

#define PAGE_SIZE 4096
#define PAGE_MASK (~(PAGE_SIZE - 1))
#define PAGE_ALIGN_DOWN(x) ((uint64_t)(x) & PAGE_MASK)
#define PAGE_ALIGN_UP(x) (((uint64_t)(x) + PAGE_SIZE - 1) & PAGE_MASK)

typedef uint64_t page_table_entry_t;
typedef struct __attribute((packed))
{
    page_table_entry_t entries[512];
} page_table_t;

void init_paging(void);