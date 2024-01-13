#include <stdint.h>
#include "vga.h"
#include "terminal.h"
#include "shell.h"

void InstallGDT();

void main()
{
    InstallGDT();
    InitTerminal();
    Term_PrintString("GDT installed!");
    Term_PrintString("\nKernel loaded!");
    InitShell();
    ShellHandleCommands();
}

// Read a byte from the specified port
unsigned char port_byte_in(unsigned short port)
{
    unsigned char result;
    /* Inline assembler syntax
     * The source and destination registers are switched from NASM
     * '"=a" (result)'; set '=' the C variable '(result)' to the value of register e'a'x
     * '"d" (port)': map the C variable '(port)' into e'd'x register
     * Inputs and outputs are separated by colons
     */
    __asm__("in %%dx, %%al" : "=a"(result) : "d"(port));
    return result;
}

// Set a byte on the specified port
void port_byte_out(unsigned short port, unsigned char data)
{
    __asm__("out %%al, %%dx" : : "a"(data), "d"(port));
}

unsigned short port_word_in(unsigned short port)
{
    unsigned short result;
    __asm__("in %%dx, %%ax" : "=a"(result) : "d"(port));
    return result;
}

void port_word_out(unsigned short port, unsigned short data)
{
    __asm__("out %%ax, %%dx" : : "a"(data), "d"(port));
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