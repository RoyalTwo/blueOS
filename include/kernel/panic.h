#include <stdint.h>
#pragma once

typedef struct
{
    uint64_t cr2;

    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;

    uint64_t rbp;
    uint64_t rdi;
    uint64_t rsi;

    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t rax;

    uint64_t interrupt_number;
    uint64_t error_code;

    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;

    uint64_t rsp;
    uint64_t ss;
} InterruptFrame;

typedef enum
{
    DIVIDE_ERROR_EXCEPTION = 0,
    DEBUG_EXCEPTION = 1,
    BREAKPOINT_EXCEPTION = 3,
    OVERFLOW_EXCEPTION = 4,
    BOUND_RANGE_EXCEEDED_EXCEPTION = 5,
    INVALID_OPCODE_EXCEPTION = 6,
    DEVICE_NOT_AVAILABLE_EXCEPTION = 7,
    DOUBLE_FAULT_EXCEPTION = 8,
    COPROCESSOR_SEGMENT_OVERRUN_EXCEPTION = 9,
    INVALID_TSS_EXCEPTION = 10,
    SEGMENT_NOT_PRESENT_EXCEPTION = 11,
    STACK_SEGMENT_FAULT_EXCEPTION = 12,
    GENERAL_PROTECTION_EXCEPTION = 13,
    PAGE_FAULT_EXCEPTION = 14,
    MATH_FAULT_EXCEPTION = 16,
    ALIGNMENT_CHECK_EXCEPTION = 17,
    MACHINE_CHECK_EXCEPTION = 18,
    SIMD_FLOATING_POINT_EXCEPTION = 19,
    VIRTUALIZATION_EXCEPTION = 20,
} ExceptionType;

void PANIC(char *message);