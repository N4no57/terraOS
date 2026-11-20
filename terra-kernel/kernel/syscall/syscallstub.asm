[bits 64]
global syscall_stub

extern syscall_handler
extern syscall_stack


syscall_stub:
    mov [user_stack], rsp
    mov rsp, syscall_stack

    call syscall_handler

    mov rsp, [user_stack]

    sysret

user_stack:
    dq 0