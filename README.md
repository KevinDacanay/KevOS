# KevOS: A Minimal x86 Operating System

## Overview
KevOS is a custom-built operating system designed for educational and experimental purposes. It is currently a **32-bit Protected Mode kernel** targeting the `i686-elf` architecture, built with a modular design that facilitates an eventual transition to 64-bit Long Mode. 

The project focuses on building a stable, Unix-like CLI environment from the ground up using freestanding C++ and a custom internal `libc`.

---

## Current Status (Implemented)
The following core components are fully operational:

### 1. Boot & Core Hardware
- **Multiboot/GRUB**: Bootable via GRUB in a Protected Mode environment.
- **GDT (Global Descriptor Table)**: Custom flat memory model segments established.
- **IDT (Interrupt Descriptor Table)**: Full interrupt routing for all 256 gates.
- **PIC Remapping**: Hardware interrupts correctly routed to the 0x20-0x2F range.
- **ISRs & IRQs**: Exception handling and hardware interrupt management.

### 2. Drivers & Input
- **VGA Text Driver**: Supports 80x25 terminal output with newline processing and automatic scrolling.
- **Keyboard Driver**: Interrupt-driven scancode translation with support for **Shift**, **Caps Lock**, and **Backspace**.
- **Internal libc**: Freestanding implementation of `stdio` and `string` functions.

---

## Planned Functionalities (Roadmap)
Detailed task tracking is maintained in `docs/plans.md`.

- **Keyboard Line Discipline**: Buffering input for shell processing.
- **Physical Memory Manager (PMM)**: Bitmap-based page allocation.
- **Virtual Memory Manager (VMM)**: Paging and higher-half kernel mapping.
- **Preemptive Multitasking**: Round-robin scheduler and context switching.
- **Shell Engine**: Interactive command parsing (`help`, `clear`, `ls`).
- **x86_64 Transition**: Moving to Long Mode and 64-bit execution.

---

## Project Structure
```text
OperatingSystemsProject/
├── kernel/
│   ├── arch/i386/        # Arch-specific code (GDT, IDT, Drivers)
│   ├── include/          # Kernel headers
│   └── kernel/           # Main kernel logic
├── libc/                 # Custom freestanding C library
├── docs/                 # Architecture, design, and roadmap documentation
└── *.sh                  # Build and emulation scripts
```

---

## Sources & Credits
This project stands on the shoulders of the OS development community. Significant inspiration, architectural patterns, and examples have been drawn from:
- OSDev Wiki: The primary technical reference for x86 hardware and kernel protocols.
- **"Operating Systems: A Practical Approach" by Robert Switzer**: Used as a foundational textbook for understanding kernel design and implementation logic.

---

## Building KevOS
The system requires an `i686-elf` cross-compiler.

1. **Build Headers**: `./headers.sh`
2. **Compile**: `./build.sh`
3. **Run in QEMU**: `./qemu.sh`
