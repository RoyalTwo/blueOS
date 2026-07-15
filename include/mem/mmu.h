#pragma once

#include <stdint.h>

#define PHY_TO_VIRT(physical) (physical + kernel.hhdm_offset) // Requires <kernel/kernel.h> included
#define PAGE_ADDR_MASK 0x000FFFFFFFFFF000ULL

#define PAGE_SIZE 4096
#define PAGE_MASK (~(PAGE_SIZE - 1))
#define PAGE_ALIGN_DOWN(x) ((uint64_t)(x) & PAGE_MASK)
#define PAGE_ALIGN_UP(x) (((uint64_t)(x) + PAGE_SIZE - 1) & PAGE_MASK)

#define PAGE_PRESENT (1ULL << 0)
#define PAGE_WRITABLE (1ULL << 1)
#define PAGE_USER (1ULL << 2)
#define PAGE_HUGE (1ULL << 7)
#define PAGE_NX (1ULL << 63)

#define PAGE_PAT (1ULL << 7)
#define PAGE_PWT (1ULL << 3)
#define PAGE_PCD (1ULL << 4)
#define PAGE_STRONG_UC (PAGE_PAT | PAGE_PWT | PAGE_PCD)

typedef uint64_t page_table_entry_t;
typedef struct __attribute((packed))
{
    page_table_entry_t entries[512];
} page_table_t;

void init_paging(void);
uint64_t vmm_allocate_page(page_table_t *pml4, uint64_t vaddr, uint64_t flags);
// Return value is 0 for success, 1 for error
int paging_map_pages(page_table_t *pml4, uint64_t virt_address, uint64_t phys_address, uint64_t num_pages, uint64_t flags);