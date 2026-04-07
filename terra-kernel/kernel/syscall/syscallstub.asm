[bits 64]
global syscall_stub

extern syscall_handler
extern kernel_stack_pointer

syscall_stub:
    mov rsp, [kernel_stack_pointer]
    call syscall_handler
    sysret