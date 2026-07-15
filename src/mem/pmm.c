#include <stdint.h>
#include <stdbool.h>
#include <printf.h>
#include <mem/mmu.h>
#include <kernel/kernel.h>
#include <kernel/panic.h>
#include <kernel/kstring.h>

struct bump_allocator
{
    uint64_t current;
    uint64_t end;
};

static struct bump_allocator bumpPMM;

struct bitmap_allocator
{
    uint8_t *bitmap;
    uint64_t bitmap_size_bytes;
    uint64_t bitmap_size_pages;
    uint64_t last_search_index;
    uint64_t bitmap_phy_addr;
    uint64_t mem_total_pages;
};

static struct bitmap_allocator PMM;

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
uint64_t bump_alloc_pages(uint64_t num_pages)
{
    if (num_pages == 0)
    {
        return 0;
    }

    uint64_t size = num_pages * PAGE_SIZE;

    if (bumpPMM.current == 0 || bumpPMM.end == 0)
    {
        return 0;
    }

    if (bumpPMM.current + size > bumpPMM.end)
    {
        return 0;
    }

    uint64_t page = bumpPMM.current;
    bumpPMM.current += size;

    return page;
}

void create_bump_pmm_for_size(uint64_t needed_pages)
{
    uint64_t needed_size = needed_pages * PAGE_SIZE;

    bumpPMM.current = 0;
    bumpPMM.end = 0;

    for (uint32_t i = 0; i < kernel.memmap->entry_count; i++)
    {
        struct limine_memmap_entry *e = kernel.memmap->entries[i];

        if (e->type != LIMINE_MEMMAP_USABLE)
            continue;

        uint64_t start = PAGE_ALIGN_UP(e->base);
        uint64_t end = PAGE_ALIGN_DOWN(e->base + e->length);

        if (start < PAGE_SIZE)
            start = PAGE_SIZE;

        if (end <= start)
            continue;

        if (end - start < needed_size)
            continue;

        bumpPMM.current = start;
        bumpPMM.end = end;

        return;
    }

    PANIC("create_bump_pmm: no usable range large enough for bitmap");
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

static inline void bm_set_page(uint64_t page)
{
    PMM.bitmap[page / 8] |= (1 << (page % 8)); // byte is page / 8, bit is page % 8
}

static inline void bm_clear_page(uint64_t page)
{
    PMM.bitmap[page / 8] &= ~(1 << (page % 8));
}

static inline bool bm_test_page(uint64_t page)
{
    return PMM.bitmap[page / 8] & (1 << (page % 8));
}

// Takes in physical address
void pmm_reserve_range(uint64_t base, uint64_t length)
{
    uint64_t base_aligned = PAGE_ALIGN_DOWN(base);
    uint64_t stop_aligned = PAGE_ALIGN_UP(base + length);
    size_t num_pages = (stop_aligned - base_aligned) / PAGE_SIZE;
    size_t current_physical = base_aligned;
    for (size_t i = 0; i < num_pages; i++)
    {
        size_t current_page_bm = GET_PAGE_NUM_FROM_PHY(current_physical);
        bm_set_page(current_page_bm);
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
        bm_clear_page(current_page_bm);
        current_physical += PAGE_SIZE;
    }
}

// Returns PHYSICAL address
uint64_t pmm_alloc_page()
{
    // Start from last search index, more likely to find an empty page
    for (uint64_t i = PMM.last_search_index; i < PMM.mem_total_pages; i++)
    {
        if (!bm_test_page(i))
        {
            bm_set_page(i);
            PMM.last_search_index = i + 1;
            return i * PAGE_SIZE;
        }
    }

    // Start from beginning if last search index couldn't help
    for (uint64_t i = 1; i < PMM.last_search_index; i++)
    {
        if (!bm_test_page(i))
        {
            bm_set_page(i);
            PMM.last_search_index = i + 1;
            return i * PAGE_SIZE;
        }
    }

    return 0; // out of physical memory
}

void pmm_free_page(uint64_t physical)
{
    if (physical == 0)
        PANIC("pmm_free_page: Tried to free physical page 0!");

    if (physical & (PAGE_SIZE - 1))
        PANIC("pmm_free_page: Tried to free unaligned physical address!");

    uint64_t page = physical / PAGE_SIZE;

    if (page >= PMM.mem_total_pages)
        PANIC("pmm_free_page: Tried to free page outside physical memory!");

    if (!bm_test_page(page))
        PANIC("pmm_free_page: Double free physical page!");

    bm_clear_page(page);

    if (page < PMM.last_search_index)
        PMM.last_search_index = page;
}

void bitmap_initial_mark_free()
{
    uint64_t entry_count = kernel.memmap->entry_count;
    struct limine_memmap_entry **entries = kernel.memmap->entries;
    for (size_t i = 0; i < entry_count; i++)
    {
        if (entries[i]->type == LIMINE_MEMMAP_USABLE) // TODO: Reclaim bootloader reclaimable
            pmm_free_range(entries[i]->base, entries[i]->length);
    }
}

struct bitmap_allocator create_bitmap(uint64_t num_phy_pages, bool verbose)
{
    uint64_t bitmap_size_bytes = (num_phy_pages + 7) / 8; // + 7 rounds up
    uint64_t bitmap_size_pages = PAGE_ALIGN_UP(bitmap_size_bytes) / PAGE_SIZE;

    if (verbose)
    {
        printf("PMM: total physical pages = %d\n", num_phy_pages);
        printf("PMM: bitmap size bytes = %d\n", bitmap_size_bytes);
        printf("PMM: bitmap size pages = %d\n", bitmap_size_pages);
        printf("PMM: bump range = 0x%p - 0x%p\n",
               bumpPMM.current,
               bumpPMM.end);
    }

    uint64_t bitmap_phys = bump_alloc_pages(bitmap_size_pages);
    if (verbose)
        printf("PMM: bitmap_phys = 0x%p\n", bitmap_phys);
    if (bitmap_phys == 0)
        PANIC("create_bitmap: bump alloc returned 0!");
    uint8_t *pmm_bitmap = (uint8_t *)PHY_TO_VIRT(bitmap_phys);
    memset(pmm_bitmap, 0xFF, bitmap_size_bytes); // 0xFF marks every page as USED
    return (struct bitmap_allocator){
        .bitmap = pmm_bitmap,
        .bitmap_size_bytes = bitmap_size_bytes,
        .bitmap_size_pages = bitmap_size_pages,
        .bitmap_phy_addr = bitmap_phys,
        .last_search_index = 1,
        .mem_total_pages = num_phy_pages,
    };
}

void init_pmm()
{
    printf("Initializing PMM...");
    print_memmap();
    uint64_t num_pages_mem = get_physical_num_pages();
    uint64_t bitmap_size_bytes = (num_pages_mem + 7) / 8;
    uint64_t bitmap_size_pages = PAGE_ALIGN_UP(bitmap_size_bytes) / PAGE_SIZE;
    create_bump_pmm_for_size(bitmap_size_pages); // Only used to initially allocate space for bitmap

    bool verbose = false;
    PMM = create_bitmap(num_pages_mem, verbose);
    bitmap_initial_mark_free();
    pmm_reserve_range(PMM.bitmap_phy_addr, PMM.bitmap_size_pages * PAGE_SIZE); // Reserve itself
    pmm_reserve_range(0, PAGE_SIZE);                                           // Returning physical address 0 should look like failure
    printf(BGRN "Done!\n" WHT);
}