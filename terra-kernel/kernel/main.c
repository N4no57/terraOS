void kernel_main(void) __attribute__((section(".text")));
void kernel_main(void) {
    unsigned short *VGA_MEMORY = (unsigned short*)0xB8000;
    const char message[] = "Hello, TerraOS!\0";
    for (int i = 0; message[i] != '\0'; i++) {
        VGA_MEMORY[i] = 0xF000 | message[i]; // white text on black background
    }
    while (1) {
        // halt
    }
}