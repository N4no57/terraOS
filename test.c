__attribute__((naked))
void _start(void) {
    asm("syscall");

    while (1) {}
}