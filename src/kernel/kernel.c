#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <cpu/gdt.h>
#include <cpu/idt.h>
#include <cpu/cpu.h>
#include <drivers/serial.h>
#include <printf.h>
#include <kernel/tty.h>
#include <kernel/bootutils.h>
#include <mem/paging.h>

// Kernel entry point
// TODO: Clean up this file
void kmain(void)
{
    // Ensure the bootloader actually understands our base revision (see spec)
    if (LIMINE_BASE_REVISION_SUPPORTED == false)
    {
        HALT();
    }

    // Ensure we got a framebuffer
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1)
    {
        HALT();
    }

    // Fetch the first framebuffer
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
    init_serial();
    printf(CLR);
    gdt_init();
    idt_init();
    tty_init(framebuffer);
    print_string("Framebuffer initialized.\n");
    print_string("Here's a second line.\n");
    init_paging();

    // Kernel should never exit
    HALT();
}