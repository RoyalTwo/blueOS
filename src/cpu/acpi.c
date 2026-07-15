#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <printf.h>
#include <kernel/panic.h>
#include <kernel/kernel.h>
#include <kernel/kstring.h>
#include <mem/mmu.h>

// Stage One ACPI parsing
// All we want to parse for now are the boot tables.
// Specifically, we want to parse the MADT to get the APICs,
// so that we can implement APIC support and hardware interrupt handling
// (timers, keyboard, etc)

typedef struct __attribute__((packed))
{
    char signature[8]; // "RSD PTR "
    uint8_t checksum;  // Checksum of first 20 bytes
    char oem_id[6];
    uint8_t revision;
    uint32_t rsdt_address;

    // ACPI 2.0+, NOT AVAILABLE IF REVISION != 2
    uint32_t length;
    uint64_t xsdt_address;
    uint8_t extended_checksum; // Checksum of entire structure
    uint8_t reserved[3];
} rsdp_t;

typedef struct __attribute__((packed))
{
    char signature[4];
    uint32_t length; // Entire table size, including this header
    uint8_t revision;
    uint8_t checksum;
    char oem_id[6];
    char oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} acpi_sdt_header_t;

typedef struct __attribute__((packed))
{
    acpi_sdt_header_t header;

    // Variable-length array of 64-bit physical addresses
    uint64_t entries[];
} xsdt_t; // USED WITH ACPI rev2

typedef struct __attribute__((packed))
{
    acpi_sdt_header_t header;

    // Variable-length array of 64-bit physical addresses
    uint32_t entries[];
} rsdt_t; // USED WITH ACPI rev0

typedef struct __attribute__((packed))
{
    uint8_t type;
    uint8_t length; // The length of this record including this header
} madt_apic_entry_header_t;

typedef struct __attribute__((packed))
{
    acpi_sdt_header_t header;

    uint32_t local_apic_address;
    uint32_t flags;

    uint8_t entries[];
} madt_t;

enum madt_entry_types
{
    PROCESSOR_LOCAL_APIC = 0,
    IO_APIC = 1,
    IO_APIC_INTERRUPT_SOURCE_OVERRIDE = 2,
    IO_APIC_NONMASKABLE_INTERRUPT_SOURCE = 3,
    LOCAL_APIC_NONMASKABLE_INTERRUPTS = 4,
    LOCAL_APIC_ADDRESS_OVERRIDE = 5,
    PROCESSOR_LOCAL_X2APIC = 9,
};

// TODO: Move to separate CPU file
static void cpu_get_msr(uint32_t msr, uint32_t *lo, uint32_t *hi)
{
    asm volatile("rdmsr" : "=a"(*lo), "=d"(*hi) : "c"(msr));
}

static void cpu_set_msr(uint32_t msr, uint32_t lo, uint32_t hi)
{
    asm volatile("wrmsr" : : "a"(lo), "d"(hi), "c"(msr));
}

size_t xsdt_entry_count(const xsdt_t *XSDT)
{
    if (XSDT->header.length < sizeof(acpi_sdt_header_t))
        return 0;

    return (XSDT->header.length - sizeof(acpi_sdt_header_t)) / sizeof(uint64_t);
}
size_t rsdt_entry_count(const rsdt_t *RSDT)
{
    if (RSDT->header.length < sizeof(acpi_sdt_header_t))
        return 0;

    return (RSDT->header.length - sizeof(acpi_sdt_header_t)) / sizeof(uint32_t);
}

static bool is_correct_revision(uint8_t revision)
{
    if (revision == 0) // ACPI version 1.0, acceptable
        return true;
    else if (revision == 2) // ACPI version 2.0+, what we want
        return true;
    else
        return false;
}

static bool checksum_valid(const void *table, size_t length)
{
    const uint8_t *bytes = table;
    uint8_t sum = 0;
    bool all_zeros = true;

    for (size_t i = 0; i < length; i++)
    {
        if (bytes[i] != 0)
            all_zeros = false;
        sum += bytes[i];
    }

    return (sum == 0 && !all_zeros);
}

bool validate_rsdp(rsdp_t *RSDP)
{
    uint8_t revision = RSDP->revision;
    if (!is_correct_revision(revision))
        return false;
    if (!checksum_valid(RSDP, 20)) // first 20 bytes must be verified separately
        return false;

    if (revision == 2)
    {
        // ACPI rev2 supports the XSDT which needs to be verified separately
        if (!checksum_valid(RSDP, RSDP->length))
            return false;
        if (RSDP->length < sizeof(rsdp_t)) // don't read ahead if the structure is malformed (ACPI rev2 only)
            return false;
    }

    return true;
}

bool validate_xsdt(xsdt_t *XSDT)
{
    if (!is_correct_revision(XSDT->header.revision))
        return false;
    if (XSDT->header.length < sizeof(acpi_sdt_header_t))
        return false;
    if (!checksum_valid(XSDT, XSDT->header.length))
        return false;

    return true;
}
bool validate_rsdt(rsdt_t *RSDT)
{
    if (RSDT->header.length < sizeof(acpi_sdt_header_t))
        return false;
    if (!checksum_valid(RSDT, RSDT->header.length))
        return false;

    return true;
}

acpi_sdt_header_t *xsdt_find_table(
    const xsdt_t *XSDT,
    const char signature[4])
{
    if (XSDT == NULL || signature == NULL)
        return NULL;

    size_t entry_count = xsdt_entry_count(XSDT);

    for (size_t i = 0; i < entry_count; i++)
    {
        uintptr_t physical_address = XSDT->entries[i];

        acpi_sdt_header_t *table =
            (acpi_sdt_header_t *)PHY_TO_VIRT(physical_address);

        if (strncmp(table->signature, signature, 4) == 0)
            return table;
    }

    return NULL;
}
acpi_sdt_header_t *rsdt_find_table(
    const rsdt_t *RSDT,
    const char signature[4])
{
    if (RSDT == NULL || signature == NULL)
        return NULL;

    size_t entry_count = rsdt_entry_count(RSDT);

    for (size_t i = 0; i < entry_count; i++)
    {
        uintptr_t physical_address = RSDT->entries[i];

        acpi_sdt_header_t *table =
            (acpi_sdt_header_t *)PHY_TO_VIRT(physical_address);

        if (strncmp(table->signature, signature, 4) == 0)
            return table;
    }

    return NULL;
}

// Returns true if parsed successfully, false if malformed
bool parse_madt(const madt_t *MADT)
{
    if (MADT == NULL || MADT->header.length < sizeof(madt_t))
        return false;

    const uint8_t *current = MADT->entries;
    const uint8_t *end =
        (const uint8_t *)MADT + MADT->header.length;

    printf("-MADT-\n");
    printf("Core Local APIC Address: 0x%p\n", MADT->local_apic_address);
    while (current < end)
    {
        size_t remaining = (size_t)(end - current);

        if (remaining < sizeof(madt_apic_entry_header_t))
            return false;

        const madt_apic_entry_header_t *entry =
            (const madt_apic_entry_header_t *)current;

        if (entry->length < sizeof(madt_apic_entry_header_t) ||
            entry->length > remaining)
        {
            return false;
        }

        printf("Entry:\n");
        switch (entry->type)
        {
        case PROCESSOR_LOCAL_APIC:
            printf(" Type: PROCESSOR_LOCAL_APIC\n");
            break;

        case IO_APIC:
            printf(" Type: IO_APIC\n");
            break;

        case IO_APIC_INTERRUPT_SOURCE_OVERRIDE:
            printf(" Type: IO_APIC_INTERRUPT_SOURCE_OVERRIDE\n");
            break;

        case IO_APIC_NONMASKABLE_INTERRUPT_SOURCE:
            printf(" Type: IO_APIC_NONMASKABLE_INTERRUPT_SOURCE\n");
            break;

        case LOCAL_APIC_NONMASKABLE_INTERRUPTS:
            printf(" Type: LOCAL_APIC_NONMASKABLE_INTERRUPTS\n");
            break;

        case LOCAL_APIC_ADDRESS_OVERRIDE:
            printf(" Type: LOCAL_APIC_ADDRESS_OVERRIDE\n");
            break;

        case PROCESSOR_LOCAL_X2APIC:
            printf(" Type: PROCESSOR_LOCAL_X2APIC\n");
            break;

        default:
            // Skip
            printf(" Type: UNKNOWN\n");
            break;
        }

        current += entry->length;
    }

    return current == end;
}

madt_t *get_madt(rsdp_t *RSDP)
{
    int revision = RSDP->revision;
    madt_t *MADT = NULL;
    if (revision == 0)
    {
        uint32_t rsdt_phy = RSDP->rsdt_address;
        rsdt_t *RSDT = (rsdt_t *)PHY_TO_VIRT(rsdt_phy);
        if (!validate_rsdt(RSDT))
            PANIC("get_madt: RSDT not valid!");
        MADT = (madt_t *)rsdt_find_table(RSDT, "APIC");
        if (MADT == NULL)
            PANIC("get_madt: MADT not valid!");
    }
    if (revision == 2)
    {
        uint64_t xsdt_phy = RSDP->xsdt_address;
        xsdt_t *XSDT = (xsdt_t *)PHY_TO_VIRT(xsdt_phy);
        if (!validate_xsdt(XSDT))
            PANIC("get_madt: XSDT not valid!");
        MADT = (madt_t *)xsdt_find_table(XSDT, "APIC");
        if (MADT == NULL)
            PANIC("get_madt: MADT not valid!");
    }

    return MADT;
}

void parse_rsdp()
{
    // TODO: Use CPUID to check for this
    rsdp_t *RSDP = (rsdp_t *)kernel.rsdp_address;
    if (!validate_rsdp(RSDP))
        PANIC("parse_rsdp: RSDP not valid!");
    madt_t *MADT = get_madt(RSDP);
    if (MADT == NULL)
        PANIC("parse_rsdp: MADT not valid!");
    if (!parse_madt(MADT))
        PANIC("parse_rsdp: Failed to fully parse MADT!");
}