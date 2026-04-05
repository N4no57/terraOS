#ifndef IDT_H
#define IDT_H

#include "utils.h"

// IDT entry for 64-bit mode
struct idt_entry {
    u16 offset_low;
    u16 selector;
    u8 ist;          // Interrupt Stack Table offset
    u8 type_attr;    // Type and attributes
    u16 offset_mid;
    u32 offset_high;
    u32 zero;
} __attribute__((packed));

struct idt_ptr {
    u16 limit;
    u64 base;
} __attribute__((packed));

void idt_init(void);
void idt_set_gate(int n, u64 handler);

#endif