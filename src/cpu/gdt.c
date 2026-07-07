#include <stdint.h>
#include "cpu/include/types.h"

typedef struct __attribute__((packed))
{
    uint16_t limit;
    uint16_t base0;
    uint8_t base1;
    uint8_t access;
    uint8_t granularity; // Contains limit_high AND flags
    uint8_t base2;
} gdt_entry_t;

typedef struct __attribute__((packed))
{
    uint16_t limit;
    uint16_t base0;
    uint8_t base1;
    uint8_t access;
    uint8_t granularity; // Contains limit_high AND flags
    uint8_t base2;
    uint32_t base3;
    uint32_t _reserved;
} tss_entry_t;

typedef struct __attribute__((packed))
{
    uint16_t size;
    uint64_t offset; // 64 bit on x64 machines
} gdt_descriptor_t;

typedef struct __attribute__((packed))
{
    gdt_entry_t null;
    gdt_entry_t kernel_code; // Index: 1  - IF THIS ORDER IS EVER CHANGED, CHANGE THE DEFINITIONS IN TYPES.h
    gdt_entry_t kernel_data; // Index: 2
    gdt_entry_t user_data;   // Index: 3
    gdt_entry_t user_code;   // Index: 4
    tss_entry_t tss_entry;
} gdt_t;

typedef struct __attribute__((packed))
{
    uint32_t _reserved0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rps2;
    uint64_t _reserved1;
    uint64_t ist[7];
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iomap_base;
} tss_t;

gdt_descriptor_t GDT_ptr;
gdt_t gdt;
tss_t TSS;

extern void gdt_flush(gdt_descriptor_t *);
extern void tss_load(void);

gdt_entry_t make_code_entry(uint8_t dpl)
{
    return (gdt_entry_t){
        .access = 0x9A | (dpl << 5),
        .granularity = 0x20,
    };
}
gdt_entry_t make_data_entry(uint8_t dpl)
{
    return (gdt_entry_t){
        .access = 0x92 | (dpl << 5)};
}
tss_entry_t make_tss_entry(tss_t *TSS)
{
    uintptr_t base = (uintptr_t)TSS;
    uint32_t limit = sizeof(*TSS) - 1;
    return (tss_entry_t){
        .limit = limit & 0xFFFF,

        .base0 = base & 0xFFFF,
        .base1 = (base >> 16) & 0xFF,
        .base2 = (base >> 24) & 0xFF,
        .base3 = base >> 32,

        .access = 0x89,

        .granularity = (limit >> 16) & 0x0F};
}
void gdt_init(void)
{
    TSS.iomap_base = sizeof(TSS);
    gdt.null = (gdt_entry_t){0, 0, 0, 0, 0, 0};
    gdt.kernel_code = make_code_entry(KERNEL_LVL);
    gdt.kernel_data = make_data_entry(KERNEL_LVL);
    gdt.user_code = make_code_entry(USER_LVL);
    gdt.user_data = make_data_entry(USER_LVL);
    gdt.tss_entry = make_tss_entry(&TSS);

    // Must be sure to disable interrupts beforehand
    asm("cli");
    GDT_ptr.size = sizeof(gdt_t) - 1;
    GDT_ptr.offset = (uint64_t)&gdt;

    gdt_flush(&GDT_ptr);
    tss_load();
    asm("sti");
}
