#include "idt.h"
#include "stdio.h"
#include "terminal.h"
#include "vga.h"
#include "io.h"

#define PIC1_COMMAND 0x0020
#define PIC1_DATA 0x0021
#define PIC2_COMMAND 0x00A0
#define PIC2_DATA 0x00A1

// PIC

void PIC_remap(uint8_t master_offset, uint8_t slave_offset)
{
    // Give initialization command
    port_byte_out(PIC1_COMMAND, 0x11);
    port_byte_out(PIC2_COMMAND, 0x11);
    // Give vector offsets
    port_byte_out(PIC1_DATA, master_offset);
    port_byte_out(PIC2_DATA, slave_offset);
    // Tell how the two PICs are wired together
    port_byte_out(PIC1_DATA, 0x04); // Tells master PIC there is a slave PIC at IRQ2 (0000 0100)
    port_byte_out(PIC2_DATA, 0x02); // Tells slave PIC its cascade identity (0000 0010)
    // Tell PICS to use 8086 mode
    port_byte_out(PIC1_DATA, 0x01);
    port_byte_out(PIC2_DATA, 0x01);
}

// IDT

const char *exception_messages[] =
    {
        "Division By Zero",
        "Debug",
        "Non-Maskable Interrupt",
        "Breakpoint",
        "Overflow",
        "Index Out Of Bounds",
        "Invalid Opcode",
        "Device Not Available",
        "Double Fault",
        "Coprocessor Segment Overrun", // Should never occur
        "Invalid TSS",
        "Segment Not Present",
        "Stack-Segment Fault",
        "General Protection Fault",
        "Page Fault",
        "Reserved",
        "x87 Floating-Point Exception",
        "Alignment Check",
        "Machine Check",
        "SIMD Floating-Point Exception",
        "Virtualization Exception",
        "Control Protection Exception",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Hypervisor Injection Exception",
        "VMM Communication Exception",
        "Security Exception",
        "Reserved"};

void isr_handler(registers_t regs)
{
    if (regs.int_no < 32)
    {
        // It's either an exception or a reserved interrupt
        uint8_t white_on_red = get_vga_color(VGA_COLOR_WHITE, VGA_COLOR_RED);
        uint8_t white_on_black = get_vga_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        Term_SetColor(white_on_red);
        printf("Exception");
        Term_SetColor(white_on_black);
        printf(" - %s", exception_messages[regs.int_no]);
        printf("\nSystem halted!");
        printf("\n%d:%d", regs.cs, regs.eip);
        for (;;)
            ;
    }
}

isr_t interrupt_handlers[256];

void register_interrupt_handler(uint8_t index, isr_t handler)
{
    interrupt_handlers[index] = handler;
}

void irq_handler(registers_t regs)
{
    // Reset interrupts on PICs
    if (regs.int_no >= 40)
    {
        // The slave sent the IRQ, so we have to reset both
        port_byte_out(0xA0, 0x20);
    }
    port_byte_out(0x20, 0x20);

    if (interrupt_handlers[regs.int_no] != 0)
    {
        // There's a function to handle it
        isr_t handler = interrupt_handlers[regs.int_no];
        handler(regs);
    }
}

struct IDT_Entry
{
    uint16_t offset_low;     // Offset represents address of entry point to interrupt service routine
    uint16_t selector;       // Code segment selector in GDT
    uint8_t reserved;        // Set to 0 always, it's unused
    uint8_t type_attributes; // Gate type (4 bits), DPL (2 bits), P (1 bit, should be 1 for descriptor to be valid)
    uint16_t offset_high;
} __attribute__((packed));

struct IDT_Descriptor
{
    uint16_t size;   // One less than the size of the IDT in bytes
    uint32_t offset; // Linear address of IDT
} __attribute__((packed));

struct IDT_Entry IDT[256]; // The IDT
struct IDT_Descriptor IDT_ptr;
extern void IDT_load(); // Defined in idt.asm

// Exception handlers (defined in idt_asm.asm)
extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

// IRQ handlers (keyboard, etc)
extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

void IDT_set_entry(int index, uint32_t handler, uint16_t selector, uint8_t attributes)
{
    IDT[index].offset_low = (handler & 0xFFFF); // & 0xFFFF gives last 8 bits
    IDT[index].offset_high = ((handler >> 16) & 0xFFFF);

    IDT[index].selector = selector; // 0x08 is code segment in GDT
    IDT[index].reserved = 0;        // Always 0
    IDT[index].type_attributes = attributes;
}

void InstallIDT()
{
    // TODO: Clear IDT memory to 0 before adding handlers
    IDT_ptr.size = (sizeof(struct IDT_Entry) * 256) - 1;
    IDT_ptr.offset = (uint32_t)&IDT;

    // Change PIC offset
    PIC_remap(0x20, 0x28);

    // Setup entries
    // Exceptions + Intel Reserved
    IDT_set_entry(0, (uint32_t)isr0, 0x08, 0x8E); // 0x8E = 32-bit interrupt gate, ring 0, present bit set
    IDT_set_entry(1, (uint32_t)isr1, 0x08, 0x8E);
    IDT_set_entry(2, (uint32_t)isr2, 0x08, 0x8E);
    IDT_set_entry(3, (uint32_t)isr3, 0x08, 0x8E);
    IDT_set_entry(4, (uint32_t)isr4, 0x08, 0x8E);
    IDT_set_entry(5, (uint32_t)isr5, 0x08, 0x8E);
    IDT_set_entry(6, (uint32_t)isr6, 0x08, 0x8E);
    IDT_set_entry(7, (uint32_t)isr7, 0x08, 0x8E);
    IDT_set_entry(8, (uint32_t)isr8, 0x08, 0x8E);
    IDT_set_entry(9, (uint32_t)isr9, 0x08, 0x8E);
    IDT_set_entry(10, (uint32_t)isr10, 0x08, 0x8E);
    IDT_set_entry(11, (uint32_t)isr11, 0x08, 0x8E);
    IDT_set_entry(12, (uint32_t)isr12, 0x08, 0x8E);
    IDT_set_entry(13, (uint32_t)isr13, 0x08, 0x8E);
    IDT_set_entry(14, (uint32_t)isr14, 0x08, 0x8E);
    IDT_set_entry(15, (uint32_t)isr15, 0x08, 0x8E);
    IDT_set_entry(16, (uint32_t)isr16, 0x08, 0x8E);
    IDT_set_entry(17, (uint32_t)isr17, 0x08, 0x8E);
    IDT_set_entry(18, (uint32_t)isr18, 0x08, 0x8E);
    IDT_set_entry(19, (uint32_t)isr19, 0x08, 0x8E);
    IDT_set_entry(20, (uint32_t)isr20, 0x08, 0x8E);
    IDT_set_entry(21, (uint32_t)isr21, 0x08, 0x8E);
    IDT_set_entry(22, (uint32_t)isr22, 0x08, 0x8E);
    IDT_set_entry(23, (uint32_t)isr23, 0x08, 0x8E);
    IDT_set_entry(24, (uint32_t)isr24, 0x08, 0x8E);
    IDT_set_entry(25, (uint32_t)isr25, 0x08, 0x8E);
    IDT_set_entry(26, (uint32_t)isr26, 0x08, 0x8E);
    IDT_set_entry(27, (uint32_t)isr27, 0x08, 0x8E);
    IDT_set_entry(28, (uint32_t)isr28, 0x08, 0x8E);
    IDT_set_entry(29, (uint32_t)isr29, 0x08, 0x8E);
    IDT_set_entry(30, (uint32_t)isr30, 0x08, 0x8E);
    IDT_set_entry(31, (uint32_t)isr31, 0x08, 0x8E);

    // IRQ handlers (keyboard, etc)
    IDT_set_entry(32, (uint32_t)irq0, 0x08, 0x8E);
    IDT_set_entry(33, (uint32_t)irq1, 0x08, 0x8E);
    IDT_set_entry(34, (uint32_t)irq2, 0x08, 0x8E);
    IDT_set_entry(35, (uint32_t)irq3, 0x08, 0x8E);
    IDT_set_entry(36, (uint32_t)irq4, 0x08, 0x8E);
    IDT_set_entry(37, (uint32_t)irq5, 0x08, 0x8E);
    IDT_set_entry(38, (uint32_t)irq6, 0x08, 0x8E);
    IDT_set_entry(39, (uint32_t)irq7, 0x08, 0x8E);
    IDT_set_entry(40, (uint32_t)irq8, 0x08, 0x8E);
    IDT_set_entry(41, (uint32_t)irq9, 0x08, 0x8E);
    IDT_set_entry(42, (uint32_t)irq10, 0x08, 0x8E);
    IDT_set_entry(43, (uint32_t)irq11, 0x08, 0x8E);
    IDT_set_entry(44, (uint32_t)irq12, 0x08, 0x8E);
    IDT_set_entry(45, (uint32_t)irq13, 0x08, 0x8E);
    IDT_set_entry(46, (uint32_t)irq14, 0x08, 0x8E);
    IDT_set_entry(47, (uint32_t)irq15, 0x08, 0x8E);

    IDT_load();
}
