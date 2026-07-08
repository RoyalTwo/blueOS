#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include <cpu/gdt.h>
#include <cpu/idt.h>
#include <cpu/cpu.h>
#include <printf.h>

// Set base revision to 2, recommended version
__attribute__((used, section(".requests"))) static volatile LIMINE_BASE_REVISION(6);

__attribute__((used, section(".requests"))) static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0};

// Start and end markers for Limine requests, can be moved to any C file
__attribute__((used, section(".requests_start_marker"))) static volatile LIMINE_REQUESTS_START_MARKER;
__attribute__((used, section(".requests_end_marker"))) static volatile LIMINE_REQUESTS_END_MARKER;

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
    gdt_init();
    idt_init();

    HALT();
}