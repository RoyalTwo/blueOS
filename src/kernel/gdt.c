#include <stdint.h>

struct GDT_Descriptor
{
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access;
    uint8_t granularity; // First 4 bits are limit, last 4 are flags
    uint8_t base_high;
} __attribute__((packed));

struct GDT_Ptr
{
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

struct GDT_Descriptor GDT[4];
struct GDT_Ptr gdt_ptr;
extern void GDT_flush(); // defined in boot.s

void createGDTEntry(int index, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{
    // Numbers are stored in little-endian order
    // Base address
    GDT[index].base_low = (base & 0xFFFF);
    GDT[index].base_mid = (base >> 16) & 0xFF;
    GDT[index].base_high = (base >> 24) & 0xFF;
    // Limits
    GDT[index].limit_low = (limit & 0xFFFF);
    GDT[index].granularity = ((limit >> 16) & 0x0F);

    // Granularity and Access flags
    GDT[index].granularity |= (gran & 0xF0);
    GDT[index].access = access;
}

void InstallGDT()
{
    gdt_ptr.limit = (sizeof(struct GDT_Descriptor) * 3) - 1;
    gdt_ptr.base = (uint32_t)&GDT;

    // Null descriptor
    createGDTEntry(0, 0, 0, 0, 0);
    // Code segment - index is 1, base address is 0, limit is 4 GB, access bit is ring 0, code, flags are set to 4 KB blocks using 32-bit
    createGDTEntry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    // Data segment - exactly the same as code but the descriptor type in access says it's a data segment
    createGDTEntry(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
    // TODO: Add TSS

    GDT_flush();
}