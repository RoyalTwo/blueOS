#include <stddef.h>
#include <stdint.h>
#include <kernel/kstring.h>

#define COM1 0x3f8

uint8_t inb(uint16_t port)
{
    uint8_t ret;
    asm volatile("inb %w1, %b0" : "=a"(ret) : "Nd"(port) : "memory");
    return ret;
}

void outb(uint16_t port, uint8_t val)
{
    asm volatile("outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
}

void init_serial()
{
    outb(COM1 + 1, 0x00); // Disable all interrupts
    outb(COM1 + 3, 0x80); // Enable DLAB (set baud rate divisor)
    outb(COM1 + 0, 0x03); // Set divisor to 3 (lo byte) 38400 baud
    outb(COM1 + 1, 0x00); //                  (hi byte)
    outb(COM1 + 3, 0x03); // 8 bits, no parity, one stop bit
    outb(COM1 + 2, 0xC7); // Enable FIFO, clear them, with 14-byte threshold
    outb(COM1 + 4, 0x0B); // IRQs enabled, RTS/DSR set
    outb(COM1 + 4, 0x1E); // Set in loopback mode, test the serial chip
    outb(COM1 + 0, 0xAE); // Test serial chip (send byte 0xAE and check if serial returns same byte)

    // Check if serial is faulty (i.e: not same byte as sent)
    if (inb(COM1 + 0) != 0xAE)
    {
        return;
    }

    // If serial is not faulty set it in normal operation mode
    // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    outb(COM1 + 4, 0x0F);
}

static int is_transmit_empty()
{
    return inb(COM1 + 5) & 0x20;
}

void write_serial_char(char ch)
{
    while (!is_transmit_empty())
        ;
    outb(COM1, ch);
}
void write_serial(const char *str)
{
    size_t len = strlen(str);
    for (size_t i = 0; i < len; i++)
        write_serial_char(str[i]);
}