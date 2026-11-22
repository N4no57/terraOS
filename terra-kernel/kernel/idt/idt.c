#include "../headers/idt.h"

#define IDT_ENTRIES 256

static struct idt_entry idt[IDT_ENTRIES];
static struct idt_ptr idtp;

// External ISR handlers (defined in isr.asm)
extern void isr0(void);
extern void isr1(void);
extern void isr2(void);
extern void isr3(void);
extern void isr4(void);
extern void isr5(void);
extern void isr6(void);
extern void isr7(void);
extern void isr8(void);
extern void isr9(void);
extern void isr10(void);
extern void isr11(void);
extern void isr12(void);
extern void isr13(void);
extern void isr14(void);
extern void isr15(void);
extern void isr16(void);
extern void isr17(void);
extern void isr18(void);
extern void isr19(void);
extern void isr20(void);
extern void isr21(void);
extern void isr22(void);
extern void isr23(void);
extern void isr24(void);
extern void isr25(void);
extern void isr26(void);
extern void isr27(void);
extern void isr28(void);
extern void isr29(void);
extern void isr30(void);
extern void isr31(void);
extern void irq0(void);
extern void irq1(void);

void idt_set_gate(int n, u64 handler) {
    idt[n].offset_low = handler & 0xFFFF;
    idt[n].selector = 0x18;  // Kernel code segment
    idt[n].ist = 0;
    idt[n].type_attr = 0x8E; // Present, Ring 0, Interrupt Gate
    idt[n].offset_mid = (handler >> 16) & 0xFFFF;
    idt[n].offset_high = (handler >> 32) & 0xFFFFFFFF;
    idt[n].zero = 0;
}

// Remap PIC to avoid conflicts with CPU exceptions
static void pic_remap(void) {
    // ICW1: Start initialization
    __asm__ volatile ("outb %0, %1" : : "a"((u8)0x11), "Nd"((u16)0x20));
    __asm__ volatile ("outb %0, %1" : : "a"((u8)0x11), "Nd"((u16)0xA0));

    // ICW2: Vector offsets (IRQ 0-7 -> INT 32-39, IRQ 8-15 -> INT 40-47)
    __asm__ volatile ("outb %0, %1" : : "a"((u8)0x20), "Nd"((u16)0x21));
    __asm__ volatile ("outb %0, %1" : : "a"((u8)0x28), "Nd"((u16)0xA1));

    // ICW3: PIC cascading
    __asm__ volatile ("outb %0, %1" : : "a"((u8)0x04), "Nd"((u16)0x21));
    __asm__ volatile ("outb %0, %1" : : "a"((u8)0x02), "Nd"((u16)0xA1));

    // ICW4: 8086 mode
    __asm__ volatile ("outb %0, %1" : : "a"((u8)0x01), "Nd"((u16)0x21));
    __asm__ volatile ("outb %0, %1" : : "a"((u8)0x01), "Nd"((u16)0xA1));

    // Mask all interrupts except IRQ1 (keyboard)
    __asm__ volatile ("outb %0, %1" : : "a"((u8)0xFD), "Nd"((u16)0x21));
    __asm__ volatile ("outb %0, %1" : : "a"((u8)0xFF), "Nd"((u16)0xA1));
}

void idt_init(void) {
    // Set up CPU exception handlers (0-31)
    idt_set_gate(0, (u64)isr0);
    idt_set_gate(1, (u64)isr1);
    idt_set_gate(2, (u64)isr2);
    idt_set_gate(3, (u64)isr3);
    idt_set_gate(4, (u64)isr4);
    idt_set_gate(5, (u64)isr5);
    idt_set_gate(6, (u64)isr6);
    idt_set_gate(7, (u64)isr7);
    idt_set_gate(8, (u64)isr8);
    idt_set_gate(9, (u64)isr9);
    idt_set_gate(10, (u64)isr10);
    idt_set_gate(11, (u64)isr11);
    idt_set_gate(12, (u64)isr12);
    idt_set_gate(13, (u64)isr13);
    idt_set_gate(14, (u64)isr14);
    idt_set_gate(15, (u64)isr15);
    idt_set_gate(16, (u64)isr16);
    idt_set_gate(17, (u64)isr17);
    idt_set_gate(18, (u64)isr18);
    idt_set_gate(19, (u64)isr19);
    idt_set_gate(20, (u64)isr20);
    idt_set_gate(21, (u64)isr21);
    idt_set_gate(22, (u64)isr22);
    idt_set_gate(23, (u64)isr23);
    idt_set_gate(24, (u64)isr24);
    idt_set_gate(25, (u64)isr25);
    idt_set_gate(26, (u64)isr26);
    idt_set_gate(27, (u64)isr27);
    idt_set_gate(28, (u64)isr28);
    idt_set_gate(29, (u64)isr29);
    idt_set_gate(30, (u64)isr30);
    idt_set_gate(31, (u64)isr31);

    // Remap PIC
    pic_remap();

    // Set up IRQ handlers (32+)
    idt_set_gate(32, (u64)irq0);  // Timer
    idt_set_gate(33, (u64)irq1);  // Keyboard

    // Load IDT
    idtp.limit = sizeof(idt) - 1;
    idtp.base = (u64)&idt;
    __asm__ volatile ("lidt %0" : : "m"(idtp));

    // Enable interrupts
    __asm__ volatile ("sti");
}