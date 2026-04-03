void kernel_main(void) {
    unsigned short *VGA_MEMORY = (unsigned short*)0xB8000;
    const char message[] = "TerraOS - 64-bit C Kernel\0";

    for (int i = 0; message[i] != '\0'; i++) {
        VGA_MEMORY[i] = 0x0F00 | message[i]; // white text on black background
    }


    for (int i = 0; i < 80 * 25; i++) {
        VGA_MEMORY[i] = 0x0F00 | ' ';
    }

    while (1) {
        __asm__ volatile ("hlt");
    }
}