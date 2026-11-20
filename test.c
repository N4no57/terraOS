__attribute__((naked))
int main() {
    asm("syscall");

    __builtin_unreachable();
}