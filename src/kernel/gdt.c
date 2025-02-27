#include <stdint.h>

typedef struct __attribute__((packed))
{
    uint16_t limit;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access;
    uint8_t granularity; // Contains limit_high AND flags
    uint8_t base_high;
} gdt_entry_t;

typedef struct __attribute__((packed))
{
    uint16_t size;
    uint64_t offset; // 64 bit on x64 machines
} gdt_descriptor_t;

typedef struct __attribute__((packed))
{
    gdt_entry_t null;
    gdt_entry_t _64bit_code;
    gdt_entry_t _64bit_data;
    gdt_entry_t user_data;
    gdt_entry_t user_code;
} gdt_t;

gdt_t gdt = {
    {0, 0, 0, 0, 0, 0},       // null
    {0, 0, 0, 0x9a, 0xa2, 0}, // 64-bit code
    {0, 0, 0, 0x92, 0xa0, 0}, // 64-bit data
    {0, 0, 0, 0xF2, 0, 0},    // user data
    {0, 0, 0, 0xFA, 0x20, 0}, // user code
}; // TODO: ADD TSS

gdt_descriptor_t GDT_ptr;

extern void gdt_flush(gdt_descriptor_t *);
void gdt_init(void)
{
    // Must be sure to disable interrupts beforehand
    asm("cli");
    GDT_ptr.size = sizeof(gdt_t) - 1;
    GDT_ptr.offset = (uint64_t)&gdt;

    gdt_flush(&GDT_ptr);
    asm("sti");
}
