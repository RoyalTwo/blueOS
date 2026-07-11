#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <cpu/gdt.h>
#include <cpu/idt.h>
#include <cpu/cpu.h>
#include <drivers/serial.h>
#include <printf.h>
#include <kernel/tty.h>
#include <kernel/bootutils.h> // Kernel is the only file that should include this header
#include <kernel/kernel.h>
#include <mem/mmu.h>
#include <mem/pmm.h>

kernel_t kernel;

void init_kernel_data()
{
    // Might need to come back to this to change how we get the framebuffer
    kernel.framebuffer = framebuffer_request.response->framebuffers[0];
    kernel.hhdm_offset = hhdm_request.response->offset;
    kernel.kernel_pos = (struct kernel_positions){
        .physical_base = kernel_address_request.response->physical_base,
        .virtual_base = kernel_address_request.response->virtual_base};
    kernel.memmap = memmap_request.response;
}

// Kernel entry point
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

    init_kernel_data();

    init_serial();
    printf(CLR);
    init_pmm();
    // TODO: eventually, mark other parts of memory as free as well. PMM only marks Usable as free, but after initalializing paging we can reclaim more memory
    gdt_init();
    idt_init();
    tty_init(kernel.framebuffer);
    print_string("Framebuffer initialized.\n");
    print_string("Here's a second line.\n");
    init_paging();

    // Kernel should never exit
    HALT();
}