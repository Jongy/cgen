.globl gencall
gencall:
    // rdi = struct gen*
    // rsi = stack top
    // we can clobber all registers, we never return...
    mov %rdi, %rbx
    // set up stack pointer.
    mov %rsi, %rsp
    // 0(%rsp) is our return address.

    // set up arguments in registers
    // doesn't matter that much if the number of arguments is less,
    // we'll be putting garbage values in the registers but the callee
    // won't even look at them.
    mov 0x10(%rbx), %rdi
    mov 0x18(%rbx), %rsi
    mov 0x20(%rbx), %rcx
    mov 0x28(%rbx), %rdx
    mov 0x30(%rbx), %r8
    mov 0x38(%rbx), %r9

    mov 0x0(%rbx), %rax
    jmp *%rax
