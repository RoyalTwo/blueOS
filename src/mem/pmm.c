#include <stdint.h>
#include <printf.h>
#include <mem/mmu.h>
#include <kernel/kernel.h>

typedef struct
{
    uint64_t current;
    uint64_t end;
} pmm_t;

static pmm_t PMM;

void print_memmap()
{
    const char *ENTRY_TYPES[] = {"USABLE                ", "RESERVED              ", "ACPI_RECLAIMABLE      ", "ACPI_NVS              ", "BAD_MEMORY            ", "BOOTLOADER_RECLAIMABLE", "EXECUTABLE_AND_MODULES", "FRAMEBUFFER           ", "RESERVED_MAPPED       "};
    struct limine_memmap_entry **entries = kernel.memmap->entries;
    uint32_t entry_count = kernel.memmap->entry_count;
    printf("\n-Memory Map-\n");
    printf("          Type          |           Base\n");
    printf("------------------------|---------------------\n");
    for (uint32_t i = 0; i < entry_count; i++)
    {
        printf("%s  |   0x%p\n", ENTRY_TYPES[entries[i]->type], entries[i]->base);
    }
}

// Simple bump allocator, doesn't even take into account other sections
void *pmm_alloc_page()
{
    if (PMM.current >= PMM.end)
    {
        printf("alloc_page: error! reached end of section memory.\n");
        return 0;
    }
    uint64_t page = PMM.current;
    PMM.current += PAGE_SIZE;
    return (void *)page;
}

void init_pmm()
{
    // for now, just choose a section
    uint64_t entry_count = kernel.memmap->entry_count;
    struct limine_memmap_entry **entries = kernel.memmap->entries;
    for (uint32_t i = 0; i < entry_count; i++)
    {
        if (entries[i]->type == LIMINE_MEMMAP_USABLE)
        {
            PMM.current = entries[i]->base;
            PMM.end = PMM.current + entries[i]->length;
            break;
        }
    }
}