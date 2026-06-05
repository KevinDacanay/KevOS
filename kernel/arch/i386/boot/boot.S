# ========== Multiboot Header Constants ==========

.set ALIGN,     1<<0            # Bit 0: Align loaded modules on page boundaries (required by GRUB).
.set MEMINFO,   1<<1            # Bit 1: Request memory info from bootloader.
.set FLAGS,     ALIGN | MEMINFO # Flags field combines ALIGN and MEMINFO requests.
.set MAGIC,     0x1BADB002      # Multiboot magic number (must be exact for GRUB to recognize).
.set CHECKSUM,  -(MAGIC + FLAGS)# Checksum = -(magic + flags), so all 3 add to zero.

# ========== Multiboot Header ==========
# This header must appear in the first 8 KiB of the binary so that GRUB can find it.

.section .multiboot              # Place this in a section named .multiboot.
.align 4                         # Align to 4 bytes.
.long MAGIC                      # Magic number (0x1BADB002).
.long FLAGS                      # Flags (request align + memory info).
.long CHECKSUM                   # Checksum (so magic + flags + checksum == 0).

# ========== Kernel Stack ==========
# Reserve 16 KiB of space in the BSS section for a kernel stack.

.section .bss                    # BSS section = uninitialized data.
.align 16                        # Align the stack to 16 bytes (important for SSE and ABI compliance).
stack_bottom:                    # Label for bottom of the stack.
.skip 16384                      # Reserve 16 KiB = 16 * 1024 bytes = 0x4000 bytes.
stack_top:                       # Label for the top of the stack (grows downward).

# ========== Kernel Entry Point ==========
# This is the first code that executes after GRUB transfers control to your kernel.

.section .text                   # Code section.
.global _start                   # Make `_start` globally visible (GRUB looks for this label).
.type _start, @function          # Declare `_start` as a function (for debugging/symbol info).
_start:
    # Set the stack pointer (esp) to the top of our reserved stack.
    movl $stack_top, %esp

    # Push the Multiboot magic and info pointer for kernel_main
    # GRUB puts the magic in eax and the pointer in ebx
    pushl %ebx
    pushl %eax

    # Jump to the main kernel function (C++ kernel_main defined in kernel.cpp).
    call kernel_main

    # If kernel_main returns (should never happen), halt the CPU safely.
    cli                          # Clear interrupts (disable further hardware interrupts).
1:  hlt                          # Halt the CPU.
    jmp 1b                       # Infinite loop: jump to label 1 again.

.size _start, . - _start         # Define size of _start function (used by debuggers/linkers).