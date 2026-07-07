#include <kernel/panic.h>
#include <printf.h>
#include <cpu/cpu.h>

void handle_exception(InterruptFrame frame)
{
    printf(BRED "                ### KERNEL PANIC! ###\n" WHT);
    printf("    An exception has occurred and execution cannot continue.\n");
    printf(BWHT "- Debug Info -\n" WHT);
    printf("    Interrupt Index: %d\n", frame.interrupt_number);
    printf("    Error Code: %d\n", frame.error_code);
    printf(BWHT "- Register Dump -\n" WHT);
    printf("    CR2: " YEL "0x%p\n" WHT, frame.cr2);
    printf("    RSP: " YEL "0x%p" WHT "   |   RAX: " YEL "0x%p\n" WHT, frame.rsp, frame.rax);
    printf("    RSI: " YEL "0x%p" WHT "   |   RBX: " YEL "0x%p\n" WHT, frame.rsi, frame.rbx);
    printf("    RDI: " YEL "0x%p" WHT "   |   RCX: " YEL "0x%p\n" WHT, frame.rdi, frame.rcx);
    printf("    RBP: " YEL "0x%p" WHT "   |   RDX: " YEL "0x%p\n" WHT, frame.rbp, frame.rdx);
    printf("    R8:  " YEL "0x%p" WHT "   |   R9:  " YEL "0x%p\n" WHT, frame.r8, frame.r9);
    printf("    R10: " YEL "0x%p" WHT "   |   R11: " YEL "0x%p\n" WHT, frame.r10, frame.r11);
    printf("    R12: " YEL "0x%p" WHT "   |   R13: " YEL "0x%p\n" WHT, frame.r12, frame.r13);
    printf("    R14: " YEL "0x%p" WHT "   |   R15: " YEL "0x%p\n" WHT, frame.r14, frame.r15);
    printf("    -----------------------------------------------------\n");
    printf("    RIP:    " YEL "0x%p\n" WHT, frame.rip);
    printf("    RFLAGS: " YEL "0x%p\n" WHT, frame.rflags);
    printf("    CS:     " YEL "0x%p\n" WHT, frame.cs);
    printf("    SS:     " YEL "0x%p\n" WHT, frame.ss);
    if ((frame.ss & 0b11) != (frame.cs & 0b11))
    {
        printf("SS and CS ring levels do not match up.\n");
    }
    else
    {
        printf("Exception occurred in ring %i\n", frame.ss & 0b11);
    }
    printf("SS w/o ring: %x\n", frame.ss & ~3);
    printf("CS w/o ring: %x\n", frame.cs & ~3);

    printf(BWHT "This machine will now halt. Please reboot.\n" WHT);

    HALT();
}