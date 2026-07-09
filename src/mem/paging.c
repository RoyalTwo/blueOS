#include <stdint.h>
#include <printf.h>
#include <mem/paging.h>
#include <kernel/kstring.h>
#include <limine.h>

#define PAGE_PRESENT (1ULL << 0)
#define PAGE_WRITABLE (1ULL << 1)
#define PAGE_USER (1ULL << 2)
#define PAGE_HUGE (1ULL << 7)
#define PAGE_NX (1ULL << 63)

#define PHY_TO_VIRT(physical) (physical + hhdm_offset)

__attribute__((used, section(".requests"))) static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0};

__attribute__((used, section(".requests"))) static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0};

typedef struct
{
    uint64_t current;
    uint64_t end;
} pmm_t;

static pmm_t PMM;

// Simple bump allocator, doesn't even take into account other sections
void *alloc_page()
{
    if (PMM.current >= PMM.end)
        return 0;
    uint64_t page = PMM.current;
    PMM.current += 4096;
    return (void *)page;
}

void print_memmap()
{
    const char *ENTRY_TYPES[] = {"USABLE                ", "RESERVED              ", "ACPI_RECLAIMABLE      ", "ACPI_NVS              ", "BAD_MEMORY            ", "BOOTLOADER_RECLAIMABLE", "EXECUTABLE_AND_MODULES", "FRAMEBUFFER           ", "RESERVED_MAPPED       "};
    struct limine_memmap_entry **entries = memmap_request.response->entries;
    uint32_t entry_count = memmap_request.response->entry_count;
    printf("\n-Memory Map-\n");
    printf("          Type          |           Base\n");
    printf("------------------------|---------------------\n");
    for (uint32_t i = 0; i < entry_count; i++)
    {
        printf("%s  |   0x%p\n", ENTRY_TYPES[entries[i]->type], entries[i]->base);
    }
}

typedef uint64_t page_table_entry_t;
typedef struct __attribute((packed))
{
    page_table_entry_t entries[512];
} page_table_t;

static inline uintptr_t page_address(page_table_entry_t entry)
{
    return entry & 0x000FFFFFFFFFF000ULL;
}
static page_table_t *PML4;

uint64_t virt_to_physical(void *virt_address, uint64_t hhdm_offset)
{
    uint64_t addr = (uint64_t)virt_address;
    size_t pml4_i = (addr >> 39) & 0x1FF;
    size_t pdpt_i = (addr >> 30) & 0x1FF;
    size_t pd_i = (addr >> 21) & 0x1FF;
    size_t pt_i = (addr >> 12) & 0x1FF;
    size_t page_offset = addr & 0xFFFU;

    page_table_entry_t pml4_entry = PML4->entries[pml4_i];
    if (!(pml4_entry & PAGE_PRESENT))
    {
        printf("Error: PML4 entry not present!");
        return 0;
    }
    page_table_t *PDPT = (page_table_t *)PHY_TO_VIRT(page_address(pml4_entry));
    page_table_entry_t pdpt_entry = PDPT->entries[pdpt_i];
    if (!(pdpt_entry & PAGE_PRESENT))
    {
        printf("Error: PDPT entry not present!");
        return 0;
    }
    page_table_t *PD = (page_table_t *)PHY_TO_VIRT(page_address(pdpt_entry));
    page_table_entry_t pd_entry = PD->entries[pd_i];
    if (!(pd_entry & PAGE_PRESENT))
    {
        printf("Error: PDPT entry not present!");
        return 0;
    }
    page_table_t *PT = (page_table_t *)PHY_TO_VIRT(page_address(pd_entry));
    page_table_entry_t pt_entry = PT->entries[pt_i];
    if (!(pt_entry & PAGE_PRESENT))
    {
        printf("Error: PT entry not present!");
        return 0;
    }
    uint64_t page_addr = page_address(pt_entry);
    uint64_t phy_addr = page_addr + page_offset;
    return phy_addr;
}

static int test = 123;

void init_paging(void)
{
    uint64_t hhdm_offset = hhdm_request.response->offset;
    uint64_t cr3_val = 0;
    asm volatile("mov %%cr3, %0" : "=r"(cr3_val));
    uint64_t pml4_addr = cr3_val + hhdm_offset;
    printf("PML4 Virtual Address: 0x%p\n", pml4_addr);
    PML4 = (page_table_t *)pml4_addr;

    struct limine_memmap_entry **entries = memmap_request.response->entries;
    uint32_t entry_count = memmap_request.response->entry_count;
    // print_memmap();
    // for now, just choose a section
    for (uint32_t i = 0; i < entry_count; i++)
    {
        if (entries[i]->type == LIMINE_MEMMAP_USABLE)
        {
            PMM.current = entries[i]->base;
            PMM.end = PMM.current + entries[i]->length;
            break;
        }
    }
    printf("Virtual Address: 0x%p\n", &test);
    uint64_t phy_addr = virt_to_physical(&test, hhdm_offset);
    printf("Physical Address: 0x%p\n", phy_addr);
}