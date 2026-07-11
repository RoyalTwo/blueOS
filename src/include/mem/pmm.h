#pragma once

void print_memmap();
uint64_t pmm_alloc_page(); // Returns physical page, not virtual
void pmm_free_page(uint64_t physical);
void pmm_free_range(uint64_t base, uint64_t length);
void pmm_reserve_range(uint64_t base, uint64_t length);
void init_pmm();