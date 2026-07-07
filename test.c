#include <stdio.h>
#include <stdint.h>

void print_binary8(unsigned char num) {
    // Loop 8 times for an 8-bit variable
    for (int i = 7; i >= 0; i--) {
        // Shift bit to position 0, then mask out everything else
        putchar(((num >> i) & 1) ? '1' : '0');
    }
    putchar('\n');
}
void print_binary16(uint16_t num) {
    // Start with a mask for the highest bit (2^15)
    for (int i = 15; i >= 0; i--) {
        // Shift bit to the 1s place and isolate it
        uint16_t bit = (num >> i) & 1;
        printf("%u", bit);
        
        // Optional: add a space after every 4 bits for readability
        if (i % 4 == 0 && i != 0) {
            printf(" ");
        }
    }
    printf("\n");
}
typedef enum
{
    KERNEL_LVL = 0,
    USER_LVL = 3,
} CODE_PRIVILEGE_LEVEL;

typedef enum {
    INTERRUPT_GATE = 0xE, // 0b1110
    TRAP_GATE = 0xF, // 0b1111
} IDT_GATE_TYPE;

static inline uint16_t merge_ist_access(uint8_t ist, uint8_t access)
{
    uint16_t output = ist;
    output = ((uint16_t)access << 8) | output;
    return output;
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

int main(void)
{
    uint8_t test_access1 = get_access(INTERRUPT_GATE, KERNEL_LVL); // SHOULD BE: 1000 1110
    uint8_t test_access2 = get_access(TRAP_GATE, KERNEL_LVL); // SHOULD BE: 1000 1111
    uint8_t test_access3 = get_access(INTERRUPT_GATE, USER_LVL); // SHOULD BE: 1110 1110
    uint8_t test_access4 = get_access(TRAP_GATE, USER_LVL); // SHOULD BE: 1110 1111

    print_binary8(test_access1);
    print_binary8(test_access2);
    print_binary8(test_access3);
    print_binary8(test_access4);
    printf("\n");
    uint16_t test_merge = merge_ist_access(0b00000111, test_access1); // SHOULD BE: 1000 1110 0000 0111
    print_binary16(test_merge);
    return 0;
}