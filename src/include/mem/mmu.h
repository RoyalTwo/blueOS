#pragma once

#define PHY_TO_VIRT(physical) (physical + kernel.hhdm_offset) // Requires <kernel/kernel.h> included
#define PAGE_ADDR_MASK 0x000FFFFFFFFFF000ULL

#define PAGE_SIZE 4096

void init_paging(void);