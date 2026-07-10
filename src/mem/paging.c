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

#define PHY_TO_VIRT(physical) (physical + HHDM_OFFSET)
#define PAGE_ADDR_MASK 0x000FFFFFFFFFF000ULL

#define PAGE_SIZE 4096

__attribute__((used, section(".requests"))) static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0};

__attribute__((used, section(".requests"))) static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0};

__attribute__((used, section(".requests"))) static volatile struct limine_kernel_address_request kernel_address_request = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0};

typedef struct
{
    uint64_t current;
    uint64_t end;
} pmm_t;

static pmm_t PMM;
static uint64_t HHDM_OFFSET;

// Simple bump allocator, doesn't even take into account other sections
void *alloc_page()
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

uint64_t virt_to_physical(page_table_t *pml4, void *virt_address)
{
    uint64_t addr = (uint64_t)virt_address;
    size_t pml4_i = (addr >> 39) & 0x1FF;
    size_t pdpt_i = (addr >> 30) & 0x1FF;
    size_t pd_i = (addr >> 21) & 0x1FF;
    size_t pt_i = (addr >> 12) & 0x1FF;
    size_t page_offset = addr & 0xFFFU;

    page_table_entry_t pml4_entry = pml4->entries[pml4_i];
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

int map_pages(page_table_t *pml4, uint64_t virt_address, uint64_t phys_address, uint64_t num_pages, uint64_t flags)
{
    if (num_pages == 0)
        return 0;
    uint64_t table_flags = PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER; // TODO: Handle permissions. Not every page should be USER perhaps?

    uint64_t addr = (uint64_t)virt_address;
    size_t pml4_i = (addr >> 39) & 0x1FF;
    size_t pdpt_i = (addr >> 30) & 0x1FF;
    size_t pd_i = (addr >> 21) & 0x1FF;
    size_t pt_i = (addr >> 12) & 0x1FF;

    for (; pml4_i < 512; pml4_i++) // Fill up every page we can if necessary
    {
        page_table_entry_t pml4_entry = pml4->entries[pml4_i];
        if (!(pml4_entry & PAGE_PRESENT))
        {
            page_table_entry_t page = (page_table_entry_t)alloc_page();
            memset((void *)PHY_TO_VIRT(page), 0, PAGE_SIZE);
            page = page | table_flags;
            pml4->entries[pml4_i] = page;
            pml4_entry = page;
        }
        page_table_t *PDPT = (page_table_t *)PHY_TO_VIRT(page_address(pml4_entry));
        for (; pdpt_i < 512; pdpt_i++) // Fill up every page directory pointer entry until we need a new pml4 entry
        {

            page_table_entry_t pdpt_entry = PDPT->entries[pdpt_i];
            if (!(pdpt_entry & PAGE_PRESENT))
            {
                page_table_entry_t page = (page_table_entry_t)alloc_page();
                memset((void *)PHY_TO_VIRT(page), 0, PAGE_SIZE);
                page = page | table_flags;
                PDPT->entries[pdpt_i] = page;
                pdpt_entry = page;
            }

            page_table_t *PD = (page_table_t *)PHY_TO_VIRT(page_address(pdpt_entry));
            for (; pd_i < 512; pd_i++) // Fill up every page directory entry until we need a new page directory pointer entry for a new page directory
            {
                page_table_entry_t pd_entry = PD->entries[pd_i];
                if (!(pd_entry & PAGE_PRESENT))
                {
                    page_table_entry_t page = (page_table_entry_t)alloc_page();
                    memset((void *)PHY_TO_VIRT(page), 0, PAGE_SIZE);
                    page = page | table_flags;
                    PD->entries[pd_i] = page;
                    pd_entry = page;
                }
                // TODO: might need to worry about page boundaries?

                page_table_t *PT = (page_table_t *)PHY_TO_VIRT(page_address(pd_entry));
                for (; pt_i < 512; pt_i++) // Fill up every page table entry until we need a new page directory entry for a new page table
                {
                    PT->entries[pt_i] = (phys_address & PAGE_ADDR_MASK) | flags;
                    num_pages--;
                    phys_address += PAGE_SIZE;
                    if (num_pages == 0)
                        return 0;
                }
                pt_i = 0;
            }
            pd_i = 0;
        }
        pdpt_i = 0;
    }
    return 1;
}

void map_sections(page_table_t *pml4, uint64_t memmap_entry_count, struct limine_memmap_entry **memmap_entries)
{
    for (uint64_t i = 0; i < memmap_entry_count; i++)
    {
        struct limine_memmap_entry *current = memmap_entries[i];
        uint64_t entry_type = current->type;
        if (entry_type == LIMINE_MEMMAP_USABLE || entry_type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE || entry_type == LIMINE_MEMMAP_KERNEL_AND_MODULES || entry_type == LIMINE_MEMMAP_FRAMEBUFFER)
        {
            uint64_t num_pages = current->length / PAGE_SIZE;
            uint64_t virt_addr = current->base + HHDM_OFFSET;
            uint64_t phy_addr = current->base;
            uint64_t flags = PAGE_PRESENT | PAGE_WRITABLE;
            printf("Mapping memory entry %d (type %d) with (%d pages, virtual_address = 0x%p, physical_address = 0x%p)...", i, entry_type, num_pages, virt_addr, phy_addr);
            if (entry_type == LIMINE_MEMMAP_FRAMEBUFFER)
                flags |= PAGE_USER;
            int error = map_pages(pml4, virt_addr, phy_addr, num_pages, flags);
            if (!error)
                printf(BGRN "Done!\n" WHT);
            else
                printf(BRED "FAILED\n" WHT);
        }
    }
}

extern uint64_t l_kernel_start[];          // Virtual start of kernel memory, defined in linker script
extern uint64_t l_kernel_writable_start[]; // Virtual start of writable kernel memory, defined in linker script
extern uint64_t l_kernel_end[];            // Virtual end of kernel memory, defined in linker script

uint64_t kernel_start = (uint64_t)l_kernel_start;
uint64_t kernel_writable_start = (uint64_t)l_kernel_writable_start;
uint64_t kernel_end = (uint64_t)l_kernel_end;

void map_kernel(page_table_t *pml4, uint64_t kernel_physical_base, uint64_t kernel_virtual_base)
{
    // Map from the start of the kernel to where the writable section starts with the present flag
    // TODO: Might need to align addresses on pages?
    int num_of_pages = (kernel_writable_start - kernel_start) / PAGE_SIZE;
    uint64_t phy_addr_kernel_start = kernel_physical_base + (kernel_start - kernel_virtual_base); // where limine loaded the kernel physically + (offset of kernel_start inside kernel image)
    printf("Mapping read-only kernel section with (%d pages, virtual_address = 0x%p, physical_address = 0x%p)...", num_of_pages, kernel_start, phy_addr_kernel_start);
    int error = map_pages(pml4, kernel_start, phy_addr_kernel_start, num_of_pages, PAGE_PRESENT);
    if (!error)
        printf(BGRN "Done!\n" WHT);
    else
        printf(BRED "FAILED\n" WHT);
    // Then map from the writable section start to the end of the kernel with present and write flags
    num_of_pages = (kernel_end - kernel_writable_start) / PAGE_SIZE;
    uint64_t phy_addr_kernel_writeable_start = kernel_physical_base + (kernel_writable_start - kernel_virtual_base);
    printf("Mapping writable kernel section with (%d pages, virtual_address = 0x%p, physical_address = 0x%p)...", num_of_pages, kernel_writable_start, phy_addr_kernel_writeable_start);
    error = map_pages(pml4, kernel_writable_start, phy_addr_kernel_writeable_start, num_of_pages, PAGE_PRESENT | PAGE_WRITABLE);
    if (!error)
        printf(BGRN "Done!\n" WHT);
    else
        printf(BRED "FAILED\n" WHT);
}

uint64_t get_cr3_pml4_address()
{
    uint64_t cr3_val = 0;
    asm volatile("mov %%cr3, %0" : "=r"(cr3_val));
    return cr3_val + HHDM_OFFSET;
}

static int test = 123;

void init_paging(void)
{
    HHDM_OFFSET = hhdm_request.response->offset;
    uint64_t pml4_addr = get_cr3_pml4_address();
    printf("PML4 Virtual Address: 0x%p\n", pml4_addr);
    page_table_t *PML4 = (page_table_t *)pml4_addr;

    struct limine_memmap_entry **entries = memmap_request.response->entries;
    struct limine_kernel_address_response *kernel_positions = kernel_address_request.response;
    uint32_t entry_count = memmap_request.response->entry_count;
    print_memmap();
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
    // print_memmap();
    map_sections(PML4, entry_count, entries);
    map_kernel(PML4, kernel_positions->physical_base, kernel_positions->virtual_base);
    printf("Attempting to map virtual address 0x%p to physical address...", &test);
    uint64_t phy_addr = virt_to_physical(PML4, &test);
    printf(" Virtual address: 0x%p\n", phy_addr);
}