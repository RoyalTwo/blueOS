#include <stdint.h>
#include <printf.h>
#include <mem/mmu.h>
#include <mem/pmm.h>
#include <kernel/kstring.h>
#include <kernel/kernel.h>
#include <kernel/panic.h>
#include <limine.h>

#define PAGE_PRESENT (1ULL << 0)
#define PAGE_WRITABLE (1ULL << 1)
#define PAGE_USER (1ULL << 2)
#define PAGE_HUGE (1ULL << 7)
#define PAGE_NX (1ULL << 63)

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
        printf("Error: PD entry not present!");
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

int paging_map_pages(page_table_t *pml4, uint64_t virt_address, uint64_t phys_address, uint64_t num_pages, uint64_t flags)
{
    if (num_pages == 0)
        return 0;
    if (virt_address & (PAGE_SIZE - 1))
    {
        printf("Virtual address is not page aligned!\n");
        return 1;
    }
    if (phys_address & (PAGE_SIZE - 1))
    {
        printf("Physical address is not page aligned!\n");
        return 1;
    }
    uint64_t table_flags = PAGE_PRESENT | PAGE_WRITABLE;
    if (flags & PAGE_USER)
        table_flags |= PAGE_USER;

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
            page_table_entry_t page = (page_table_entry_t)pmm_alloc_page();
            if (page == 0)
                return 1;
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
                page_table_entry_t page = (page_table_entry_t)pmm_alloc_page();
                if (page == 0)
                    return 1;
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
                    page_table_entry_t page = (page_table_entry_t)pmm_alloc_page();
                    if (page == 0)
                        return 1;
                    memset((void *)PHY_TO_VIRT(page), 0, PAGE_SIZE);
                    page = page | table_flags;
                    PD->entries[pd_i] = page;
                    pd_entry = page;
                }

                page_table_t *PT = (page_table_t *)PHY_TO_VIRT(page_address(pd_entry));
                for (; pt_i < 512; pt_i++) // Fill up every page table entry until we need a new page directory entry for a new page table
                {
                    PT->entries[pt_i] = (phys_address & PAGE_ADDR_MASK) | flags | PAGE_PRESENT;
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
#define LIMINE_MEMMAP_RESERVED_MAPPED 8 // Inexplicably, limine.h does not have this defined even though it should
                                        // TODO: Figure that out?
        if (entry_type == LIMINE_MEMMAP_USABLE ||
            entry_type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE ||
            entry_type == LIMINE_MEMMAP_KERNEL_AND_MODULES ||
            entry_type == LIMINE_MEMMAP_FRAMEBUFFER ||
            entry_type == LIMINE_MEMMAP_ACPI_RECLAIMABLE ||
            entry_type == LIMINE_MEMMAP_ACPI_NVS ||
            entry_type == LIMINE_MEMMAP_RESERVED_MAPPED)
        {
            // TODO: eventually reclaim bootloader memory, first map stack and GDT + IDT
            uint64_t phy_start = PAGE_ALIGN_DOWN(current->base);
            uint64_t phy_end = PAGE_ALIGN_UP(current->base + current->length);
            uint64_t num_pages = (phy_end - phy_start) / PAGE_SIZE;

            uint64_t virt_addr = phy_start + kernel.hhdm_offset;
            uint64_t phy_addr = phy_start;
            uint64_t flags = PAGE_PRESENT | PAGE_WRITABLE;
            printf(RARROW "Mapping memory entry %d (type %d) with (%d pages, virtual_address = 0x%p, physical_address = 0x%p)...", i, entry_type, num_pages, virt_addr, phy_addr);
            if (entry_type == LIMINE_MEMMAP_ACPI_NVS || entry_type == LIMINE_MEMMAP_ACPI_RECLAIMABLE)
                flags = PAGE_PRESENT; // not writable
            int error = paging_map_pages(pml4, virt_addr, phy_addr, num_pages, flags);
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
    uint64_t virt_start = PAGE_ALIGN_DOWN(kernel_start);
    uint64_t virt_end = PAGE_ALIGN_UP(kernel_writable_start);
    int num_of_pages = (virt_end - virt_start) / PAGE_SIZE;
    uint64_t phy_start = kernel_physical_base + (virt_start - kernel_virtual_base); // where limine loaded the kernel physically + (offset of kernel_start inside kernel image)

    printf(RARROW "Mapping read-only kernel section with (%d pages, virtual_address = 0x%p, physical_address = 0x%p)...", num_of_pages, virt_start, phy_start);
    int error = paging_map_pages(pml4, virt_start, phy_start, num_of_pages, PAGE_PRESENT);
    if (!error)
        printf(BGRN "Done!\n" WHT);
    else
        printf(BRED "FAILED\n" WHT);

    // Then map from the writable section start to the end of the kernel with present and write flags
    virt_start = PAGE_ALIGN_DOWN(kernel_writable_start);
    virt_end = PAGE_ALIGN_UP(kernel_end);
    num_of_pages = (virt_end - virt_start) / PAGE_SIZE;
    uint64_t phy_writable_start = kernel_physical_base + (virt_start - kernel_virtual_base);

    printf(RARROW "Mapping writable kernel section with (%d pages, virtual_address = 0x%p, physical_address = 0x%p)...", num_of_pages, virt_start, phy_writable_start);
    error = paging_map_pages(pml4, virt_start, phy_writable_start, num_of_pages, PAGE_PRESENT | PAGE_WRITABLE);
    if (!error)
        printf(BGRN "Done!\n" WHT);
    else
        printf(BRED "FAILED\n" WHT);
}

uint64_t get_cr3_pml4_address()
{
    uint64_t cr3_val = 0;
    asm volatile("mov %%cr3, %0" : "=r"(cr3_val));
    return cr3_val + kernel.hhdm_offset;
}

static inline void load_cr3(uint64_t pml4_phys)
{
    asm volatile("mov %0, %%cr3" ::"r"(pml4_phys) : "memory");
}

// Returns physical address of mapped page
// Returns 0 if failed
uint64_t vmm_allocate_page(page_table_t *pml4, uint64_t vaddr, uint64_t flags)
{
    size_t pml4_i = (vaddr >> 39) & 0x1FF;
    size_t pdpt_i = (vaddr >> 30) & 0x1FF;
    size_t pd_i = (vaddr >> 21) & 0x1FF;
    size_t pt_i = (vaddr >> 12) & 0x1FF;

    uint64_t table_flags = PAGE_PRESENT | PAGE_WRITABLE;
    if (flags & PAGE_USER)
        table_flags |= PAGE_USER;

    page_table_entry_t pml4_entry = pml4->entries[pml4_i];
    if (!(pml4_entry & PAGE_PRESENT))
    {
        page_table_entry_t page = (page_table_entry_t)pmm_alloc_page();
        if (page == 0)
            return 0;
        memset((void *)PHY_TO_VIRT(page), 0, PAGE_SIZE);
        page = page | table_flags;
        pml4->entries[pml4_i] = page;
        pml4_entry = page;
    }
    page_table_t *PDPT = (page_table_t *)PHY_TO_VIRT(page_address(pml4_entry));
    page_table_entry_t pdpt_entry = PDPT->entries[pdpt_i];
    if (!(pdpt_entry & PAGE_PRESENT))
    {
        page_table_entry_t page = (page_table_entry_t)pmm_alloc_page();
        if (page == 0)
            return 0;
        memset((void *)PHY_TO_VIRT(page), 0, PAGE_SIZE);
        page = page | table_flags;
        PDPT->entries[pdpt_i] = page;
        pdpt_entry = page;
    }
    page_table_t *PD = (page_table_t *)PHY_TO_VIRT(page_address(pdpt_entry));
    page_table_entry_t pd_entry = PD->entries[pd_i];
    if (!(pd_entry & PAGE_PRESENT))
    {
        page_table_entry_t page = (page_table_entry_t)pmm_alloc_page();
        if (page == 0)
            return 0;
        memset((void *)PHY_TO_VIRT(page), 0, PAGE_SIZE);
        page = page | table_flags;
        PD->entries[pd_i] = page;
        pd_entry = page;
    }
    page_table_t *PT = (page_table_t *)PHY_TO_VIRT(page_address(pd_entry));
    page_table_entry_t pt_entry = PT->entries[pt_i];
    if (!(pt_entry & PAGE_PRESENT))
    {
        page_table_entry_t page = (page_table_entry_t)pmm_alloc_page();
        if (page == 0)
            return 0;
        memset((void *)PHY_TO_VIRT(page), 0, PAGE_SIZE);
        page = page | table_flags;
        PT->entries[pt_i] = page;
        pt_entry = page;
    }
    uint64_t page_addr = page_address(pt_entry);
    return page_addr;
}

static int test = 123;

// TODO: Unmapping pages
// TODO: Might need to switch kernel stack
void init_paging(void)
{
    printf("Transferring paging...\n");
    uint64_t new_pml4_phys = pmm_alloc_page();
    if (new_pml4_phys == 0)
        PANIC("init_paging: Out of physical memory!");
    page_table_t *PML4 = (void *)PHY_TO_VIRT(new_pml4_phys);
    memset((void *)PML4, 0, PAGE_SIZE);

    struct limine_memmap_entry **entries = kernel.memmap->entries;
    uint32_t entry_count = kernel.memmap->entry_count;
    map_sections(PML4, entry_count, entries);
    map_kernel(PML4, kernel.kernel_pos.physical_base, kernel.kernel_pos.virtual_base);

    load_cr3(new_pml4_phys);
    kernel.PML4 = PML4;

    printf(RARROW "Attempting to map virtual address 0x%p to physical address...", &test);
    uint64_t phy_addr = virt_to_physical(PML4, &test);
    printf(" Virtual address: 0x%p.\n", phy_addr);
    printf(BGRN "...Done!\n" WHT);
}