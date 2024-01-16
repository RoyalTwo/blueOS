#include <stdint.h>
#include "vga.h"
#include "terminal.h"
#include "shell.h"
#include "stdio.h"
#include "idt.h"
#include "timer.h"

void InstallGDT();

void main()
{
    InstallGDT();
    InitTerminal();
    InstallIDT();
    printf("Installed GDT!");
    printf("\n");
    printf("Kernel loaded!");
    printf("\n");
    ShellHandleCommands();
    __asm__("sti");
}

// GDT
struct GDT_Entry
{
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access;
    uint8_t granularity; // Contains limit_high AND flags
    uint8_t base_high;
} __attribute__((packed));

struct GDT_Descriptor
{
    uint16_t limit;
    unsigned int base;
} __attribute__((packed));

struct GDT_Entry GDT[3]; // The GDT
struct GDT_Descriptor GDT_ptr;
extern void GDT_flush(); // defined in boot.asm

void GDT_set_entry(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{
    // Setup base address
    GDT[num].base_low = (base & 0xFFFF);
    GDT[num].base_mid = (base >> 16) & 0xFF;
    GDT[num].base_high = (base >> 24) & 0xFF;

    // Setup limits
    GDT[num].limit_low = (limit & 0xFFFF);
    GDT[num].granularity = ((limit >> 16) & 0x0F);

    // Setup granularity and access flags
    GDT[num].granularity |= (gran & 0xF0);
    GDT[num].access = access;
}

void InstallGDT()
{
    GDT_ptr.limit = (sizeof(struct GDT_Entry) * 3) - 1;
    GDT_ptr.base = (unsigned int)&GDT;

    // NULL descriptor
    GDT_set_entry(0, 0, 0, 0, 0);
    // Code segment - index is 1, base address is 0, limit is 4 GB, access bit is ring 0, code, flags are set to 4 KB blocks using 32-bit
    GDT_set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    // Data segment - exactly the same as code but the descriptor type in access says it's a data segment
    GDT_set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
    // TODO: Add TSS

    GDT_flush();
}
