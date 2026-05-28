# Project Overview: BareBonesOS

## Vision
BareBonesOS is a custom-built, 32-bit x86 operating system designed with a modular "MeatyOS" philosophy. While it currently operates in 32-bit Protected Mode, the underlying architecture is being prepared for an eventual transition to 64-bit Long Mode. The goal is to create a stable, CLI-driven environment with a robust custom `libc`.

## Core Objectives
1. **Stability First**: Implementing reliable GDT and IDT structures to prevent Triple Faults.
2. **Modularity**: Separation of architecture-specific code (`kernel/arch/i386`) from generic kernel logic.
3. **Unix-like Feel**: A terminal-based interface supporting standard commands (`ls`, `echo`, `clear`).
4. **Education**: Serving as a transparent platform to understand low-level systems programming.

## Getting Started
The project requires a cross-compiler environment.
- **Target**: `i686-elf`
- **Compiler**: `i686-elf-gcc` / `i686-elf-g++`
- **Build System**: A collection of Shell scripts (`build.sh`, `qemu.sh`, `iso.sh`) located in the root directory.

### Build Pipeline
1. **Headers**: `./headers.sh` installs kernel and libc headers into the `sysroot`.
2. **Compilation**: `./build.sh` compiles assembly stubs, drivers, and kernel logic into the `bin/` directory.
3. **ISO Generation**: `./iso.sh` wraps the kernel in a GRUB-compliant ISO image.
4. **Emulation**: `./qemu.sh` boots the system in QEMU.

## Current Milestone: The Interactive Shell
We have successfully transitioned from a passive "print-only" kernel to an "interrupt-aware" kernel. The system currently recognizes keyboard scancodes and translates them to ASCII, displaying them via the VGA text-mode driver.

## Logical Evolution
The project follows a "bottom-up" development approach:
1. **Boot**: Handled by Multiboot and `boot.S`.
2. **Environment**: GDT for segment management.
3. **Interrupts**: IDT and PIC remapping for hardware communication.
4. **Drivers**: VGA and Keyboard.
5. **Logic**: (Next) Buffering input and executing commands.