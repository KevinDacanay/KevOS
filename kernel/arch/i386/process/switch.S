.section .text
.global switch_to

switch_to:
    # Save current task's state
    push %ebp
    push %ebx
    push %esi
    push %edi

    # Save current stack pointer into the 'old_esp' pointer
    mov 20(%esp), %eax # First argument: uint32_t* old_esp
    mov %esp, (%eax)

    # Load the new stack pointer
    mov 24(%esp), %esp # Second argument: uint32_t new_esp

    # Restore new task's state
    pop %edi
    pop %esi
    pop %ebx
    pop %ebp
    ret
