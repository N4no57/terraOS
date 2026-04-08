[bits 64]
global syscall_stub

extern syscall_handler

syscall_stub:
    call syscall_handler
    sysret