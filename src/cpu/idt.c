#include <stdint.h>
#include "cpu/include/idt.h"
#include "cpu/include/types.h"

typedef struct __attribute((packed)) 
{
    uint16_t offset0;
    uint16_t segment;
    uint8_t ist;
    uint8_t access;
    uint16_t offset1;
    uint32_t offset2;
    uint32_t _reserved;
} idt_entry_t;

typedef struct __attribute__((packed)) 
{
    uint16_t size;
    uint64_t offset;
} idt_descriptor_t;

idt_descriptor_t IDT_ptr;
static idt_entry_t* IDT; // Total Maximum: 255

typedef struct
{
    uint8_t stack_index; // 0 for disabled, 1-7 for IST entry (n)
    uint16_t segment;    // Should almost always be KERNEL_CODE_SELECTOR
    IDT_GATE_TYPE gate_type;
    CODE_PRIVILEGE_LEVEL dpl;
} idt_options_t;

typedef struct {
    uint16_t offset0;
    uint16_t offset1;
    uint32_t offset2;
} split_offset;
static inline split_offset get_offset(uint64_t full_offset)
{
    return (split_offset){
        .offset0 = full_offset & 0xFFFF,
        .offset1 = (full_offset >> 16) & 0xFFFF,
        .offset2 = (full_offset >> 32) & 0xFFFFFFFF,
    };
}
static inline uint8_t get_access(IDT_GATE_TYPE gate_type, CODE_PRIVILEGE_LEVEL dpl)
{
    uint8_t access = 0x00;
    access = gate_type & 0xF;
    // middle bit is 0
    access = ((dpl & 0xF) << 5) | access;
    // always set present bit
    access = 0b10000000 | access;
    return access;
}

extern void divideErrorException();
extern void debugException();
extern void breakpointException();
extern void overflowException();
extern void BOUNDRangeExceededException();
extern void invalidOpcodeException();
extern void deviceNotAvailableException();
extern void doubleFaultException();
extern void coprocessorSegmentOverrunException();
extern void invalidTSSException();
extern void segmentNotPresentException();
extern void stackSegmentFaultException();
extern void generalProtectionException();
extern void pageFaultException();
extern void mathFaultException();
extern void alignmentCheckException();
extern void machineCheckException();
extern void SIMDFloatingPointException();
extern void virtualizationException();


/*
    Creates new IDT entry with default options.
    Default options:
        Kernel Privilege Access Level
        Interrupts Disabled (Interrupt Gate, not Trap Gate)
        Interrupt Stack Table (IST) Unused
*/
idt_entry_t create_new_entry_default(void* handler)
{
    split_offset offset = get_offset((uint64_t)handler);
    uint8_t access = get_access(INTERRUPT_GATE, KERNEL_LVL);
    idt_entry_t new_entry = {
        .offset0 = offset.offset0,
        .offset1 = offset.offset1,
        .offset2 = offset.offset2,
        .ist = 0x0,
        .access = access,
        .segment = KERNEL_CODE_SELECTOR,
    };
    return new_entry;
}
idt_entry_t create_new_entry_args(void* handler, idt_options_t options)
{
    split_offset offset = get_offset((uint64_t)handler);
    uint8_t access = get_access(options.gate_type, options.dpl);
    idt_entry_t new_entry = {
        .offset0 = offset.offset0,
        .offset1 = offset.offset1,
        .offset2 = offset.offset2,
        .ist = options.stack_index,
        .access = access,
        .segment = options.segment,
    };
    return new_entry;
}

void idt_init(void) 
{
    IDT[0] = create_new_entry_default(divideErrorException);
    IDT[1] = create_new_entry_args(debugException, (idt_options_t){ .stack_index = 0, .segment = KERNEL_CODE_SELECTOR, .gate_type = TRAP_GATE, KERNEL_LVL});
    IDT[3] = create_new_entry_args(breakpointException, (idt_options_t){ .stack_index = 0, .segment = KERNEL_CODE_SELECTOR, .gate_type = TRAP_GATE, KERNEL_LVL});
    IDT[4] = create_new_entry_args(overflowException, (idt_options_t){ .stack_index = 0, .segment = KERNEL_CODE_SELECTOR, .gate_type = TRAP_GATE, KERNEL_LVL});
    IDT[5] = create_new_entry_default(BOUNDRangeExceededException);
    IDT[6] = create_new_entry_default(invalidOpcodeException);
    IDT[7] = create_new_entry_default(deviceNotAvailableException);
    IDT[8] = create_new_entry_default(doubleFaultException);
    IDT[9] = create_new_entry_default(coprocessorSegmentOverrunException);
    IDT[10] = create_new_entry_default(invalidTSSException);
    IDT[11] = create_new_entry_default(segmentNotPresentException);
    IDT[12] = create_new_entry_default(stackSegmentFaultException);
    IDT[13] = create_new_entry_default(generalProtectionException);
    IDT[14] = create_new_entry_default(pageFaultException);
    IDT[16] = create_new_entry_default(mathFaultException);
    IDT[17] = create_new_entry_default(alignmentCheckException);
    IDT[18] = create_new_entry_default(machineCheckException);
    IDT[19] = create_new_entry_default(SIMDFloatingPointException);
    IDT[20] = create_new_entry_default(virtualizationException);

    IDT_ptr.size = (sizeof(idt_entry_t) * 256) - 1;
    IDT_ptr.offset = (uint64_t)&IDT;

    asm volatile ("lidt %0" : : "m"(IDT_ptr));
}