#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "limine.h"

// Set base revision to 2, recommended version
__attribute__((used, section(".requests"))) static volatile LIMINE_BASE_REVISION(2);

__attribute__((used, section(".requests"))) static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0};

// Start and end markers for Limine requests, can be moved to any C file
__attribute__((used, section(".requests_start_marker"))) static volatile LIMINE_REQUESTS_START_MARKER;
__attribute__((used, section(".requests_end_marker"))) static volatile LIMINE_REQUESTS_END_MARKER;

static void halt(void)
{
    asm("cli");
    for (;;)
    {
        asm("hlt");
    }
}

void putpixel(struct limine_framebuffer *framebuffer, int x, int y)
{
    volatile uint32_t *fb_ptr = framebuffer->address;
    fb_ptr[(x) + (y * (framebuffer->width))] = 0xffffff;
}

void drawsquare(struct limine_framebuffer *fb, int x, int y, int length)
{
    volatile uint32_t *fb_ptr = fb->address;
    for (int i = 0; i < length; i++)
    {
        for (int j = 0; j < length; j++)
        {
            fb_ptr[(j) + (i * fb->width) + (x) + (y * fb->width)] = 0xffffffff;
        }
    }
}

void putc(struct limine_framebuffer *fb, char c)
{
    // Start at just 0, 0
}

// Kernel entry point
void kmain(void)
{
    // Ensure the bootloader actually understands our base revision (see spec)
    if (LIMINE_BASE_REVISION_SUPPORTED == false)
    {
        halt();
    }

    // Ensure we got a framebuffer
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1)
    {
        halt();
    }

    // Fetch the first framebuffer
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];

    // Note: we assume the framebuffer model is RGB with 32-bit pixels.
    drawsquare(framebuffer, 10, 10, 200);
    drawsquare(framebuffer, 10, 220, 200);

    // Kernel should never exit
    halt();
}