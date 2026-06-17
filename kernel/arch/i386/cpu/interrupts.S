/* This file contains the assembly stubs for Interrupt Service Routines (ISRs)
   and Interrupt Request (IRQ) handlers. */

.section .text

/* GDT Flush function to load the new GDT */
.global gdt_flush
gdt_flush:
    mov 4(%esp), %eax
    lgdt (%eax)
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    ljmp $0x08, $.flush
.flush:
    ret

/* Macros for generating ISR and IRQ stubs */
.macro ISR_NOERRCODE num
  .global isr\num
  isr\num:
    pushl $0
    pushl $\num
    jmp common_isr_handler
.endm

.macro ISR_ERRCODE num
  .global isr\num
  isr\num:
    pushl $\num
    jmp common_isr_handler
.endm

.macro IRQ num, target
  .global irq\num
  irq\num:
    pushl $0
    pushl $\target
    jmp common_irq_handler
.endm

/* ISRs 0-31: CPU Exceptions */
ISR_NOERRCODE 0;  ISR_NOERRCODE 1;  ISR_NOERRCODE 2;  ISR_NOERRCODE 3;
ISR_NOERRCODE 4;  ISR_NOERRCODE 5;  ISR_NOERRCODE 6;  ISR_NOERRCODE 7;
ISR_ERRCODE   8;  ISR_NOERRCODE 9;  ISR_ERRCODE   10; ISR_ERRCODE   11;
ISR_ERRCODE   12; ISR_ERRCODE   13; ISR_ERRCODE   14; ISR_NOERRCODE 15;
ISR_NOERRCODE 16; ISR_ERRCODE   17; ISR_NOERRCODE 18; ISR_NOERRCODE 19;
ISR_NOERRCODE 20; ISR_ERRCODE   21; ISR_NOERRCODE 22; ISR_NOERRCODE 23;
ISR_NOERRCODE 24; ISR_NOERRCODE 25; ISR_NOERRCODE 26; ISR_NOERRCODE 27;
ISR_NOERRCODE 28; ISR_ERRCODE   29; ISR_ERRCODE   30; ISR_NOERRCODE 31;

/* IRQs 0-15: Hardware Interrupts remapped to vectors 32-47 */
IRQ 0, 32; IRQ 1, 33; IRQ 2, 34; IRQ 3, 35; IRQ 4, 36; IRQ 5, 37; IRQ 6, 38; IRQ 7, 39;
IRQ 8, 40; IRQ 9, 41; IRQ 10, 42; IRQ 11, 43; IRQ 12, 44; IRQ 13, 45; IRQ 14, 46; IRQ 15, 47;

/* Common handler for all CPU Exceptions */
common_isr_handler:
    pushl %eax
    pushl %ecx
    pushl %edx
    pushl %ebx
    pushl %esp    /* Pushes original ESP */
    pushl %ebp
    pushl %esi
    pushl %edi

    xor %eax, %eax
    mov %ds, %ax
    pushl %eax

    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    pushl %esp
    call isr_handler
    add $4, %esp

    popl %eax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    popl %edi
    popl %esi
    popl %ebp
    add $4, %esp  /* Skip the ESP we pushed */
    popl %ebx
    popl %edx
    popl %ecx
    popl %eax

    add $8, %esp
    iret

/* Common handler for all Hardware IRQs */
common_irq_handler:
    pushl %eax
    pushl %ecx
    pushl %edx
    pushl %ebx
    pushl %esp
    pushl %ebp
    pushl %esi
    pushl %edi

    xor %eax, %eax
    mov %ds, %ax
    pushl %eax

    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    pushl %esp
    call irq_handler
    add $4, %esp

    popl %eax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    popl %edi
    popl %esi
    popl %ebp
    add $4, %esp
    popl %ebx
    popl %edx
    popl %ecx
    popl %eax

    add $8, %esp
    iret

/* System Call Interrupt stub (int 0x80) */
.global isr128
isr128:
    cli
    pushl $0
    pushl $128

    pushl %eax
    pushl %ecx
    pushl %edx
    pushl %ebx
    pushl %esp
    pushl %ebp
    pushl %esi
    pushl %edi

    mov %ds, %ax
    pushl %eax

    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    pushl %esp
    call syscall_handler
    add $4, %esp

    popl %eax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    popl %edi
    popl %esi
    popl %ebp
    add $4, %esp
    popl %ebx
    popl %edx
    popl %ecx
    popl %eax

    add $8, %esp
    iret

/*
 * @brief Jumps to User Mode (Ring 3).
 * Expects the entry point in EBX and the user stack pointer in ECX.
 */
.global jump_to_usermode
jump_to_usermode:
    mov 4(%esp), %ebx   /* Load Parameter 1 (entry point) into EBX */
    mov 8(%esp), %ecx   /* Load Parameter 2 (user stack) into ECX */

    cli
    mov $0x23, %ax      /* User Data Segment selector: Index 4, RPL 3 (0x20 | 3) */
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    pushl $0x23         /* SS (User Data) */
    pushl %ecx          /* ESP (User Stack) */
    pushf               /* EFLAGS */
    popl %eax
    orl $0x200, %eax    /* Set IF (Interrupt Flag) so interrupts are enabled in Ring 3 */
    pushl %eax
    pushl $0x1B         /* CS (User Code): Index 3, RPL 3 (0x18 | 3) */
    pushl %ebx          /* EIP (Entry Point) */
    iret