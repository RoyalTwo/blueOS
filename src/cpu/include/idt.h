#pragma once

void idt_init(void);

typedef enum {
    INTERRUPT_GATE = 0xE,
    TRAP_GATE = 0xF,
} IDT_GATE_TYPE;