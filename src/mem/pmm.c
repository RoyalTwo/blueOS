#include <stdint.h>
#include <printf.h>
#include <mem/mmu.h>
#include <kernel/kernel.h>
#include <kstring.h>

struct early_pmm
{
    uint64_t current;
    uint64_t end;
};

static struct early_pmm bumpPMM;

uint8_t *pmm_bitmap;
uint64_t *pmm_bitmap_size_bytes;

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
void *bump_alloc_pages(uint64_t num_pages)
{
    if (bumpPMM.current >= bumpPMM.end)
    {
        printf("bump_alloc_pages: error! reached end of section memory.\n");
        return 0;
    }
    uint64_t page = bumpPMM.current;
    bumpPMM.current += PAGE_SIZE * num_pages;
    return (void *)page;
}

void create_bump_pmm()
{
    // just choose a section
    uint64_t entry_count = kernel.memmap->entry_count;
    struct limine_memmap_entry **entries = kernel.memmap->entries;
    for (uint32_t i = 0; i < entry_count; i++)
    {
        if (entries[i]->type == LIMINE_MEMMAP_USABLE)
        {
            bumpPMM.current = entries[i]->base;
            bumpPMM.end = bumpPMM.current + entries[i]->length;
            break;
        }
    }
}

uint64_t get_physical_num_pages()
{
    uint64_t entry_count = kernel.memmap->entry_count;
    struct limine_memmap_entry **entries = kernel.memmap->entries;
    uint64_t highest_address = 0;
    for (uint64_t i = 0; i < entry_count; i++)
    {
        struct limine_memmap_entry *entry = entries[i];
        uint64_t end = entry->base + entry->length;
        if (end > highest_address)
            highest_address = end;
    }

    return PAGE_ALIGN_UP(highest_address) / PAGE_SIZE;
}

#define GET_PAGE_NUM_FROM_PHY(physical) (physical / PAGE_SIZE)
#define BM_GET_BYTE_FROM_PAGE(page) ((uint64_t(page / 8)))
#define BM_GET_BIT_FROM_PAGE(page) ((uint8_t(page % 8)))

uint64_t bm_get_byte_from_page(uint64_t page)
{
    return (page / 8);
}

uint8_t bm_get_bit_from_page(uint64_t page)
{
    return (uint8_t)(page % 8);
}

void bm_mark_used(uint8_t *bitmap, uint64_t byte_index, uint8_t bit_index)
{
    bitmap[byte_index] |= (1 << bit_index);
}
void bm_mark_free(uint8_t *bitmap, uint64_t byte_index, uint8_t bit_index)
{
    bitmap[byte_index] &= ~(1 << bit_index);
}

void pmm_reserve_range(uint64_t base, uint64_t length)
{
    // Takes in physical address
    uint64_t base_aligned = PAGE_ALIGN_DOWN(base);
    uint64_t stop_aligned = PAGE_ALIGN_UP(base + length);
    size_t num_pages = (stop_aligned - base_aligned) / PAGE_SIZE;
    size_t current_physical = base_aligned;
    for (size_t i = 0; i < num_pages; i++)
    {
        size_t current_page_bm = GET_PAGE_NUM_FROM_PHY(current_physical);
        bm_mark_used(pmm_bitmap, bm_get_byte_from_page(current_page_bm), bm_get_bit_from_page(current_page_bm));
        current_physical += PAGE_SIZE;
    }
}

void pmm_free_range(uint64_t base, uint64_t length)
{
    // Takes in physical address
    uint64_t base_aligned = PAGE_ALIGN_UP(base); // only free complete pages inside the usable range
    uint64_t stop_aligned = PAGE_ALIGN_DOWN(base + length);
    size_t num_pages = (stop_aligned - base_aligned) / PAGE_SIZE;
    size_t current_physical = base_aligned;
    for (size_t i = 0; i < num_pages; i++)
    {
        size_t current_page_bm = GET_PAGE_NUM_FROM_PHY(current_physical);
        bm_mark_free(pmm_bitmap, bm_get_byte_from_page(current_page_bm), bm_get_bit_from_page(current_page_bm));
        current_physical += PAGE_SIZE;
    }
}

// Returns PHYSICAL address
uint64_t pmm_alloc_page()
{
}

void pmm_free_page(uint64_t page)
{
}

void create_bitmap(uint64_t num_phy_pages)
{
    uint64_t bitmap_size_bytes = (num_phy_pages + 7) / 8; // + 7 rounds up
    uint64_t bitmap_size_pages = PAGE_ALIGN_UP(bitmap_size_bytes) / PAGE_SIZE;

    uint64_t bitmap_phys = bump_alloc_pages(bitmap_size_pages);
    pmm_bitmap = (uint8_t)PHY_TO_VIRT(bitmap_phys);
    pmm_bitmap_size_bytes = bitmap_size_bytes;
    memset(pmm_bitmap, 0xFF, bitmap_size_bytes); // 0xFF marks every page as USED
    bitmap_initial_mark_free();
    pmm_reserve_range(bitmap_phys, bitmap_size_pages * PAGE_SIZE); // Reserve itself
    pmm_reserve_range(0, PAGE_SIZE);                               // Returning physical address 0 should look like failure
}

void bitmap_initial_mark_free()
{
    uint64_t entry_count = kernel.memmap->entry_count;
    struct limine_memmap_entry **entries = kernel.memmap->entries;
    for (size_t i = 0; i < entry_count; i++)
    {
        if (entries[i]->type == LIMINE_MEMMAP_USABLE)
            pmm_free_range(entries[i]->base, entries[i]->length);
    }
}

void init_pmm()
{
    create_bump_pmm(); // Only used to initially allocate space for bitmap
    uint64_t num_pages_mem = get_physical_num_pages();
    create_bitmap(num_pages_mem);
}