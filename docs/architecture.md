# Architecture Design Document

This document details the low-level implementation logic for the i386 architecture.

## 1. Memory Layout
- **Load Address**: The kernel is loaded at `1 MiB` (0x100000) by the bootloader (GRUB).
- **VGA Buffer**: Located at `0xB8000`. The `tty.cpp` driver manages this 80x25 character grid.
- **Stack**: A 16 KiB stack is defined in `boot.S`, aligned to 16 bytes for compatibility.

## 2. Segmentation (GDT)
The Global Descriptor Table defines the memory segments the CPU can access. We use a "Flat Memory Model":
- **Null Segment (0x00)**: Required by the processor.
- **Kernel Code Segment (0x08)**: Base 0, Limit 4GB, Ring 0, Exec/Read.
- **Kernel Data Segment (0x10)**: Base 0, Limit 4GB, Ring 0, Read/Write.

*Implementation: `kernel/arch/i386/cpu/gdt.cpp`*

## 3. Interrupt Handling (IDT & PIC)
The Interrupt Descriptor Table (IDT) maps 256 interrupt gates.

### PIC Remapping
The Programmable Interrupt Controller (8259 PIC) is remapped to avoid conflicts with CPU exceptions:
- **Master PIC**: Remapped to 0x20 - 0x27.
- **Slave PIC**: Remapped to 0x28 - 0x2F.

### Interrupt Service Routines (ISRs)
- **ISRs 0-31**: Handle CPU exceptions (e.g., Divide-by-Zero, Page Fault).
- **IRQs 32-47**: Handle hardware interrupts.

*Logic Flow*:
1. Hardware triggers interrupt.
2. Assembly stub in `interrupts.S` saves registers (`pusha`).
3. Stub calls C++ handler (`isr_handler` or `irq_handler`).
4. Handler performs logic (e.g., reading keyboard port).
5. IRQ handlers send EOI (End of Interrupt) to the PIC.
6. Assembly stub restores registers and calls `iret`.

## 4. Drivers

### VGA (tty.cpp)
Supports:
- Writing characters and strings.
- Terminal scrolling when the bottom row is exceeded.
- Newline (`\n`) processing.
- Color configuration via VGA attributes.

### Keyboard (keyboard.cpp)
- **Port**: 0x60 (Data), 0x64 (Status).
- **Mapping**: Uses Scancode Set 1.
- **Logic**: Currently translates "key down" events into ASCII and pushes them directly to the terminal.

## 5. System Library (libc)
A custom freestanding `libc` is implemented to provide standard C functions without relying on a host OS.
- **Location**: `/libc/`
- **Key Functions**: `printf`, `memcpy`, `memset`, `strlen`.

## 6. Future 64-bit Scaling
The directory structure separates `arch/i386` from the main `kernel/` logic. To move to 64-bit:
1. Add `arch/x86_64`.
2. Implement 4-level Paging.
3. Update GDT to include Long Mode bits.