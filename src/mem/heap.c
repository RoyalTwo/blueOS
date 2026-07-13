#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <mem/heap.h>
#include <mem/mmu.h>
#include <printf.h>
#include <kernel/kernel.h>
#include <kernel/panic.h>

#define KERNEL_HEAP_START UINT64_C(0xffffc00000000000)
#define KERNEL_HEAP_SIZE (100ULL * 1024ULL)
#define KERNEL_HEAP_LIMIT (KERNEL_HEAP_START + KERNEL_HEAP_SIZE)

#define HEAP_ALIGNMENT 16ULL

#define ALLOCATION_MAGIC UINT64_C(0xB10CA110CA7ED123)

// A free-list node is stored at the beginning of every free region
// `size` is the total size of the free region, including this node
typedef struct freelist_node
{
    struct freelist_node *next;
    size_t size;
} freelist_node_t;

/*
 * Stored at the beginning of every allocated block.
 *
 * block_size:
 *     Total number of bytes consumed from the heap, including the header,
 *     payload, padding, and any tiny remainder absorbed by the allocation.
 *
 * requested_size:
 *     Number of payload bytes requested by the caller. This is useful for
 *     krealloc so that it does not copy alignment padding as though it were
 *     caller-owned data.
 */
typedef struct
{
    size_t block_size;
    size_t requested_size;
    uint64_t magic;
} allocation_header_t;

#define ALLOCATION_HEADER_SIZE                               \
    ((sizeof(allocation_header_t) + HEAP_ALIGNMENT - 1ULL) & \
     ~(HEAP_ALIGNMENT - 1ULL))

static freelist_node_t *free_list_head = NULL;
static bool heap_initialized = false;

static uintptr_t heap_start(void)
{
    return (uintptr_t)KERNEL_HEAP_START;
}

static uintptr_t heap_end(void)
{
    return (uintptr_t)KERNEL_HEAP_LIMIT;
}

static bool is_power_of_two(size_t value)
{
    return value != 0 && (value & (value - 1)) == 0;
}

static bool align_up_size(
    size_t value,
    size_t alignment,
    size_t *result)
{
    if (result == NULL)
        return false;

    if (!is_power_of_two(alignment))
        return false;

    size_t adjustment = alignment - 1;

    if (value > SIZE_MAX - adjustment)
        return false;

    *result = (value + adjustment) & ~adjustment;
    return true;
}

static bool ranges_overlap(
    uintptr_t a_start,
    uintptr_t a_end,
    uintptr_t b_start,
    uintptr_t b_end)
{
    return a_start < b_end && b_start < a_end;
}

static void heap_safety_checks(void)
{
    if ((KERNEL_HEAP_START % PAGE_SIZE) != 0)
    {
        PANIC("heap_safety_checks: Heap start is not page aligned!");
    }

    if ((KERNEL_HEAP_SIZE % PAGE_SIZE) != 0)
    {
        PANIC("heap_safety_checks: Heap size is not page aligned!");
    }

    if ((KERNEL_HEAP_START % HEAP_ALIGNMENT) != 0)
    {
        PANIC("heap_safety_checks: Heap start is not allocator aligned!");
    }

    if (KERNEL_HEAP_SIZE < sizeof(freelist_node_t))
    {
        PANIC("heap_safety_checks: Heap is too small!");
    }

    uintptr_t calculated_heap_limit;

    if (__builtin_add_overflow(
            (uintptr_t)KERNEL_HEAP_START,
            (uintptr_t)KERNEL_HEAP_SIZE,
            &calculated_heap_limit))
    {
        PANIC("heap_safety_checks: Heap address range overflow!");
    }

    if (calculated_heap_limit != (uintptr_t)KERNEL_HEAP_LIMIT)
    {
        PANIC("heap_safety_checks: Invalid heap limit!");
    }

    uint64_t highest_physical_end = 0;

    for (uint64_t i = 0; i < kernel.memmap->entry_count; ++i)
    {
        struct limine_memmap_entry *entry =
            kernel.memmap->entries[i];

        uint64_t entry_end;

        if (__builtin_add_overflow(
                entry->base,
                entry->length,
                &entry_end))
        {
            PANIC("heap_safety_checks: Invalid memory-map entry!");
        }

        if (entry_end > highest_physical_end)
        {
            highest_physical_end = entry_end;
        }
    }

    uintptr_t hhdm_start = (uintptr_t)kernel.hhdm_offset;
    uintptr_t hhdm_end;

    if (__builtin_add_overflow(
            hhdm_start,
            (uintptr_t)highest_physical_end,
            &hhdm_end))
    {
        PANIC("heap_safety_checks: HHDM range overflow!");
    }

    if (ranges_overlap(
            heap_start(),
            heap_end(),
            hhdm_start,
            hhdm_end))
    {
        PANIC("heap_safety_checks: Kernel heap overlaps HHDM!");
    }

    extern char l_kernel_start[];
    extern char l_kernel_end[];

    if (ranges_overlap(
            heap_start(),
            heap_end(),
            (uintptr_t)l_kernel_start,
            (uintptr_t)l_kernel_end))
    {
        PANIC("heap_safety_checks: Kernel heap overlaps kernel image!");
    }
}

// Requires the free list to remain sorted by address
static void coalesce_free_regions(void)
{
    freelist_node_t *current = free_list_head;

    while (current != NULL && current->next != NULL)
    {
        uintptr_t current_start = (uintptr_t)current;
        uintptr_t current_end;

        if (__builtin_add_overflow(
                current_start,
                current->size,
                &current_end))
        {
            PANIC("coalesce_free_regions: Free region overflow!");
        }

        uintptr_t next_start = (uintptr_t)current->next;

        if (current_end == next_start)
        {
            freelist_node_t *next = current->next;

            if (__builtin_add_overflow(
                    current->size,
                    next->size,
                    &current->size))
            {
                PANIC("coalesce_free_regions: Size overflow!");
            }

            current->next = next->next;

            /*
             * Do not advance. The enlarged region might also be adjacent
             * to the following region.
             */
        }
        else
        {
            current = current->next;
        }
    }
}

// Returns false when the region is invalid or overlaps an existing region
static bool insert_free_region(
    uintptr_t region_start,
    size_t region_size)
{
    if (region_size < sizeof(freelist_node_t))
        return false;

    if ((region_start % HEAP_ALIGNMENT) != 0)
        return false;

    if (region_start < heap_start() ||
        region_start >= heap_end())
    {
        return false;
    }

    if (region_size > heap_end() - region_start)
        return false;

    uintptr_t region_end = region_start + region_size;

    freelist_node_t *previous = NULL;
    freelist_node_t *current = free_list_head;

    while (current != NULL &&
           (uintptr_t)current < region_start)
    {
        previous = current;
        current = current->next;
    }

    /*
     * Check overlap with the preceding free region.
     */
    if (previous != NULL)
    {
        uintptr_t previous_end;

        if (__builtin_add_overflow(
                (uintptr_t)previous,
                previous->size,
                &previous_end))
        {
            PANIC("insert_free_region: Existing region overflow!");
        }

        if (previous_end > region_start)
            return false;
    }

    /*
     * Check overlap with the following free region.
     */
    if (current != NULL &&
        region_end > (uintptr_t)current)
    {
        return false;
    }

    freelist_node_t *new_node =
        (freelist_node_t *)region_start;

    new_node->size = region_size;
    new_node->next = current;

    if (previous == NULL)
    {
        free_list_head = new_node;
    }
    else
    {
        previous->next = new_node;
    }

    coalesce_free_regions();
    return true;
}

// Remove and return the first free region that can satisfy the requested block size
static freelist_node_t *take_free_region(size_t required_size)
{
    freelist_node_t *previous = NULL;
    freelist_node_t *current = free_list_head;

    while (current != NULL)
    {
        if (current->size >= required_size)
        {
            if (previous == NULL)
            {
                free_list_head = current->next;
            }
            else
            {
                previous->next = current->next;
            }

            current->next = NULL;
            return current;
        }

        previous = current;
        current = current->next;
    }

    return NULL;
}

static allocation_header_t *payload_to_header(void *ptr)
{
    return (allocation_header_t *)((uintptr_t)ptr - ALLOCATION_HEADER_SIZE);
}

static void *header_to_payload(allocation_header_t *header)
{
    return (void *)((uintptr_t)header + ALLOCATION_HEADER_SIZE);
}

static bool allocation_pointer_is_plausible(const void *ptr)
{
    if (ptr == NULL)
        return false;

    uintptr_t payload_start = (uintptr_t)ptr;

    if ((payload_start % HEAP_ALIGNMENT) != 0)
        return false;

    if (payload_start < heap_start() + ALLOCATION_HEADER_SIZE)
        return false;

    if (payload_start >= heap_end())
        return false;

    return true;
}

void *kmalloc(size_t requested_size)
{
    if (!heap_initialized)
    {
        PANIC("kmalloc: Heap has not been initialized!");
    }

    if (requested_size == 0)
        return NULL;

    size_t payload_size;

    if (!align_up_size(
            requested_size,
            HEAP_ALIGNMENT,
            &payload_size))
    {
        return NULL;
    }

    if (payload_size > SIZE_MAX - ALLOCATION_HEADER_SIZE)
        return NULL;

    size_t required_block_size =
        ALLOCATION_HEADER_SIZE + payload_size;

    freelist_node_t *region =
        take_free_region(required_block_size);

    if (region == NULL)
        return NULL;

    // Save before overwriting the free-list node with the allocation header
    uintptr_t block_start = (uintptr_t)region;
    size_t original_region_size = region->size;

    size_t excess_size =
        original_region_size - required_block_size;

    size_t allocated_block_size = required_block_size;

    // If the remainder is too small to hold a free-list node, absorb it into the allocation
    if (excess_size >= sizeof(freelist_node_t))
    {
        uintptr_t remainder_start =
            block_start + required_block_size;

        if (!insert_free_region(
                remainder_start,
                excess_size))
        {
            // This means there's an allocator bug and we should restore the free region
            if (!insert_free_region(
                    block_start,
                    original_region_size))
            {
                PANIC("kmalloc: Failed to restore free region!");
            }

            PANIC("kmalloc: Failed to insert remainder!");
        }
    }
    else
    {
        allocated_block_size = original_region_size;
    }

    allocation_header_t *header =
        (allocation_header_t *)block_start;

    header->block_size = allocated_block_size;
    header->requested_size = requested_size;
    header->magic = ALLOCATION_MAGIC;

    return header_to_payload(header);
}

void kfree(void *ptr)
{
    if (ptr == NULL)
        return;

    if (!heap_initialized)
    {
        PANIC("kfree: Heap has not been initialized!");
    }

    if (!allocation_pointer_is_plausible(ptr))
    {
        PANIC("kfree: Invalid allocation pointer!");
    }

    allocation_header_t *header =
        payload_to_header(ptr);

    if (header->magic != ALLOCATION_MAGIC)
    {
        PANIC("kfree: Invalid pointer or double free!");
    }

    uintptr_t block_start = (uintptr_t)header;
    size_t block_size = header->block_size;

    if (block_size < ALLOCATION_HEADER_SIZE)
    {
        PANIC("kfree: Corrupted allocation block size!");
    }

    if (block_size > heap_end() - block_start)
    {
        PANIC("kfree: Allocation extends beyond heap!");
    }

    // invalidate allocation
    header->magic = 0;
    header->requested_size = 0;

    if (!insert_free_region(
            block_start,
            block_size))
    {
        PANIC("kfree: Freed region overlaps existing free memory!");
    }
}

void *kcalloc(size_t count, size_t size)
{
    size_t total_size;

    if (__builtin_mul_overflow(
            count,
            size,
            &total_size))
    {
        return NULL;
    }

    if (total_size == 0)
        return NULL;

    void *ptr = kmalloc(total_size);

    if (ptr == NULL)
        return NULL;

    // could use memset
    uint8_t *bytes = (uint8_t *)ptr;

    for (size_t i = 0; i < total_size; ++i)
    {
        bytes[i] = 0;
    }

    return ptr;
}

void *krealloc(void *ptr, size_t new_size)
{
    if (ptr == NULL)
        return kmalloc(new_size);

    if (new_size == 0)
    {
        kfree(ptr);
        return NULL;
    }

    if (!heap_initialized)
    {
        PANIC("krealloc: Heap has not been initialized!");
    }

    if (!allocation_pointer_is_plausible(ptr))
    {
        PANIC("krealloc: Invalid allocation pointer!");
    }

    allocation_header_t *old_header =
        payload_to_header(ptr);

    if (old_header->magic != ALLOCATION_MAGIC)
    {
        PANIC("krealloc: Invalid allocation header!");
    }

    size_t old_requested_size =
        old_header->requested_size;

    // If the existing block already has enough payload capacity, keep it
    size_t old_payload_capacity =
        old_header->block_size - ALLOCATION_HEADER_SIZE;

    if (new_size <= old_payload_capacity)
    {
        old_header->requested_size = new_size;
        return ptr;
    }

    void *new_ptr = kmalloc(new_size);

    if (new_ptr == NULL)
        return NULL;

    size_t copy_size =
        old_requested_size < new_size
            ? old_requested_size
            : new_size;

    uint8_t *destination = (uint8_t *)new_ptr;
    const uint8_t *source = (const uint8_t *)ptr;

    for (size_t i = 0; i < copy_size; ++i)
    {
        destination[i] = source[i];
    }

    kfree(ptr);
    return new_ptr;
}

// TODO: Add slab allocator for small allocations; free-lists can be suboptimal performance-wise
void init_heap(void)
{
    if (heap_initialized)
    {
        PANIC("init_heap: Heap already initialized!");
    }

    heap_safety_checks();

    // page align
    const size_t heap_page_count =
        (KERNEL_HEAP_SIZE + PAGE_SIZE - 1) / PAGE_SIZE;

    for (size_t i = 0; i < heap_page_count; ++i)
    {
        uintptr_t virtual_address =
            heap_start() + i * PAGE_SIZE;
        if (!vmm_map_page(
                kernel.PML4,
                virtual_address,
                0))
        {
            PANIC("init_heap: Failed to map heap page!");
        }
    }

    free_list_head = NULL;

    if (!insert_free_region(
            heap_start(),
            KERNEL_HEAP_SIZE))
    {
        PANIC("init_heap: Failed to create initial free region!");
    }

    heap_initialized = true;
}