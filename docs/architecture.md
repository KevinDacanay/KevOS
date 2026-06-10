# KevOS Architecture Design Document

This document provides an extensive breakdown of KevOS's internal structure, detailing the interaction between hardware abstraction layers, interrupt logic, and the user interface.

## 1. The Boot Chain: From GRUB to C++
The boot process begins with GNU GRUB, which loads our kernel using the Multiboot protocol.

1.  **`kernel/arch/i386/boot/boot.S`**: 
    *   Defines the Multiboot header for GRUB.
    *   Establishes a 16 KiB stack.
    *   Calls `_init` to execute global C++ constructors (critical for static objects).
    *   Jumps to `kernel_main`.
2.  **`kernel/kernel/kernel.cpp`**: 
    *   The high-level entry point. It orchestrates the initialization of all sub-systems (GDT, IDT, Drivers) and finally hands over control to `shell_main`.

## 2. Core CPU Infrastructure (The Plumbing)
The "Wiring" of the kernel relies on three main components working in tandem: Segmentation, Interrupt routing, and I/O.

### 2.1 Segmentation (GDT)
**File**: `kernel/arch/i386/cpu/gdt.cpp`
Before interrupts or multitasking can function, we establish a **Flat Memory Model**. The GDT defines two primary segments:
*   **Kernel Code (0x08)**: Executable memory at Ring 0.
*   **Kernel Data (0x10)**: Writable memory at Ring 0.
The `gdt_install` function populates the table and uses assembly to reload the segment registers (`cs`, `ds`, `es`, `fs`, `gs`, `ss`).

### 2.2 The Interrupt Chain (IDT, ISR, IRQ)
KevOS uses a multi-layered approach to handle hardware and software events:

1.  **`idt.cpp`**: Defines the 256 gates in the Interrupt Descriptor Table.
2.  **`interrupts.S`**: Contains assembly stubs for every interrupt. These stubs:
    *   Push the interrupt number and error code to the stack.
    *   Save all registers (`pusha`).
    *   Call the C++ handler (`isr_handler` or `irq_handler`).
    *   Restore registers (`popa`) and return via `iret`.
3.  **`isr.cpp`**: Manages exceptions 0–31 (e.g., Page Faults). It captures the state of the CPU via the `registers` struct defined in `idt.h`.
4.  **`irq.cpp`**: Manages hardware interrupts 32–47. It maintains an array of function pointers (`irq_routines`). Drivers register themselves here using `irq_install_handler`.
5.  **`pic.cpp`**: Remaps the 8259 Programmable Interrupt Controllers. Hardware IRQs 0–15 are mapped to IDT vectors 32–47 to avoid clashing with CPU exceptions. It also handles the **EOI (End of Interrupt)** signal.

## 3. Interactive Subsystem (The Driver Stack)

### 3.1 Keyboard-to-Shell Flow
Tracing a keypress in KevOS:
1.  **Hardware**: User presses 'A'. The keyboard controller triggers **IRQ 1**.
2.  **PIC**: The PIC translates this to **IDT Vector 33** and signals the CPU.
3.  **Assembly**: The stub in `interrupts.S` saves the state and calls `irq_handler(33)`.
4.  **IRQ Manager**: `irq.cpp` looks at its table and calls the `keyboard_handler` registered by the driver.
5.  **Keyboard Driver (`keyboard.cpp`)**: 
    *   Reads the scancode from Port `0x60`.
    *   Translates scancode to ASCII ('a') using `kbd_us`.
    *   Applies modifier logic (Shift/Caps Lock).
    *   Pushes the character into a **Circular Buffer** (`kbd_buffer`).
6.  **Shell (`shell.cpp`)**:
    *   `shell_readline` calls `keyboard_getchar`.
    *   `keyboard_getchar` blocks (executes `hlt`) until the buffer is non-empty.
    *   The character is retrieved and echoed via `terminal_putchar`.

### 3.2 VGA Output (TTY)
**File**: `kernel/arch/i386/drivers/tty.cpp`
*   **Memory**: Manages the VGA buffer at `0xB8000`.
*   **Features**: Hardware cursor movement via I/O ports `0x3D4/0x3D5`, screen scrolling by shifting the 80x25 grid, and newline/backspace interpretation.
*   **Interface**: Exposed via `terminal_putchar`, which is the primary sink for `printf`.

## 4. The Library Layer (libc)
KevOS provides a freestanding `libc` located in `/libc/`. 

*   **freestanding vs hosted**: Our `libc` does not rely on kernel syscalls yet. It is compiled as `libk` for kernel use.
*   **Plumbing**: `printf` calls `putchar`, which is implemented in `libc/stdio/putchar.cpp`. In the kernel context, this `putchar` is linked to `terminal_putchar` in `tty.cpp`.
*   **Memory/String**: Standard implementations of `memcpy`, `memset`, `strcmp`, and `strlen` allow the kernel and drivers to use familiar C logic.

## 5. File Manifest & Connectivity Matrix

| Component | Files | Primary Responsibility | Connected To |
| :--- | :--- | :--- | :--- |
| **Boot** | `boot.S` | Multiboot, Stack, C++ Init | `kernel.cpp` |
| **Segments** | `gdt.cpp` | Flat Memory Model | CPU hardware |
| **Remapping** | `pic.cpp` | Hardware IRQ routing | `irq.cpp` |
| **Interrupts** | `idt.cpp`, `interrupts.S` | Gate definitions & stubs | `isr.cpp`, `irq.cpp` |
| **Managers** | `isr.cpp`, `irq.cpp` | C++ event handling | Drivers (Keyboard) |
| **Video** | `tty.cpp` | VGA buffer (0xB8000) | `shell.cpp`, `libc` |
| **Input** | `keyboard.cpp` | Scancode -> ASCII | `shell.cpp`, `irq.cpp` |
| **Interface** | `shell.cpp` | Command execution | `keyboard.cpp`, `tty.cpp` |
| **Library** | `libc/` | Standard logic | Entire Project |

## 6. Physical Memory Layout

| Address Range | Usage | Managed By |
| :--- | :--- | :--- |
| `0x00000000 - 0x000003FF` | IVT (Real Mode) | Inherited from BIOS |
| `0x00000400 - 0x000004FF` | BDA (BIOS Data Area) | BIOS |
| `0x000B8000 - 0x000B8FA0` | VGA Text Buffer | `tty.cpp` |
| `0x00100000` | Kernel Load Start | GRUB / `linker.ld` |
| `0x00100000 + kernel_size` | Kernel End / Free Memory | (Future) PMM |

## 7. Future Architectural Shifts
The current i386 implementation is designed to be swapped.
1.  **PMM Integration**: The next phase introduces `MEM-01`, which will sit between the Bootloader and the rest of the kernel, managing physical page bitmaps.
2.  **Long Mode**: Transitioning to `x86_64` will require a new `arch/x86_64` directory, replacing the 32-bit GDT/IDT/Paging logic while keeping `kernel/kernel/` generic logic intact.