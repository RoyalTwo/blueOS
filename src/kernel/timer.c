#include "timer.h"
#include "idt.h"
#include "io.h"
#include "stdio.h"

uint32_t ticks = 0;

static void timer_callback(registers_t regs)
{
    ticks++;
    printf("Tick: %d\n", ticks);
}

void InitTimer(uint32_t frequency)
{
    uint32_t divisor = 1193180 / frequency; // PIT runs at 1193180 Hz, we send the value to divide by
    // Command byte
    port_byte_out(0x43, 0x36);
    // Divisor has to be sent in bytes
    uint8_t low = (uint8_t)(divisor & 0xFF);         // Last 8 bits
    uint8_t high = (uint8_t)((divisor >> 8) & 0xFF); // First 8 bits

    port_byte_out(0x40, low);
    port_byte_out(0x40, high);
    register_interrupt_handler(IRQ0, &timer_callback);
}