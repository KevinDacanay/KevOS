# ========== Multiboot Header Constants ==========

.set ALIGN,     1<<0            # Bit 0: Align loaded modules on page boundaries (required by GRUB).
.set MEMINFO,   1<<1            # Bit 1: Request memory info from bootloader.
.set FLAGS,     ALIGN | MEMINFO # Flags field combines ALIGN and MEMINFO requests.
.set MAGIC,     0x1BADB002      # Multiboot magic number (must be exact for GRUB to recognize).
.set CHECKSUM,  -(MAGIC + FLAGS)# Checksum = -(magic + flags), so all 3 add to zero.

# ========== Paging Constants ==========
.set VIRT_BASE, 0xC0000000
.set PDE_INDEX, (VIRT_BASE >> 22)

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

# ========== Bootstrap Paging Structures ==========
# Pre-allocate page directory and one page table for the first 4MB.
.section .data
.align 4096
boot_page_directory:
    # Identity map the first 4MB (0-4MB)
    .long (boot_page_table1 - VIRT_BASE) + 0x003 # Physical address + Present + Writable
    # Fill the gap until 3GB
    .fill (PDE_INDEX - 1), 4, 0
    # Map 3GB+ to the first 4MB physical
    .long (boot_page_table1 - VIRT_BASE) + 0x003 # Physical address + Present + Writable
    # Fill the rest of the directory
    .fill (1024 - PDE_INDEX - 1), 4, 0

boot_page_table1:
    .set i, 0
    .rept 1024
        .long i + 0x003 # Identity map: Physical addr + Present + Writable
        .set i, i + 4096
    .endr

# ========== Kernel Entry Point ==========
# This is the first code that executes after GRUB transfers control to your kernel.

.section .text                   # Code section.
.global _start                   # Make `_start` globally visible (GRUB looks for this label).
.type _start, @function          # Declare `_start` as a function (for debugging/symbol info).
_start:
    # Load the physical address of the page directory into cr3
    movl $(boot_page_directory - VIRT_BASE), %ecx
    movl %ecx, %cr3

    # Enable paging by setting the PG bit (bit 31) in cr0
    movl %cr0, %ecx
    orl $0x80000000, %ecx
    movl %ecx, %cr0

    # Jump to the higher-half code
    # This absolute jump is required to transition from low physical EIP to high virtual EIP.
    lea 1f, %ecx
    jmp *%ecx

1:
    # Set the stack pointer (esp) to the top of our reserved stack.
    # This address is now in the higher-half (0xC0xxxxxx).
    movl $stack_top, %esp

    # Push the Multiboot magic and info pointer.
    # Note: multiboot_ptr is a physical address; C code must handle the conversion if needed.
    pushl %ebx
    pushl %eax

    # Jump to the main kernel function (C++ kernel_main defined in kernel.cpp).
    # We use an absolute call to ensure we stay in the higher half.
    movl $kernel_main, %ecx
    call *%ecx

    # If kernel_main returns (should never happen), halt the CPU safely.
    cli                          # Clear interrupts (disable further hardware interrupts).
1:  hlt                          # Halt the CPU.
    jmp 1b                       # Infinite loop: jump to label 1 again.

.size _start, . - _start         # Define size of _start function (used by debuggers/linkers).