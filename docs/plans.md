# Operating System Product Backlog

## SCRUM Board / Task Tracking

| Task ID | Task Description | Priority | Est. Time | Status |
| :--- | :--- | :--- | :--- | :--- |
| **CORE-01** | Global Descriptor Table (GDT) Setup | High | 4h | **Done** |
| **CORE-02** | IDT & Exception Handling (ISRs) | High | 6h | **Done** |
| **CORE-03** | PIC Remapping & IRQ Handling | High | 4h | **Done** |
| **DRV-01** | VGA Text Mode Driver (Scrolling/Newline) | High | 5h | **Done** |
| **DRV-02** | Keyboard Driver (Scancode Translation) | High | 4h | **Done** |
| **UI-01** | Keyboard Input Buffering (Line Discipline) | Highest | 4h | **Done** | 
| **DRV-04** | VGA Hardware Cursor Management | Highest | 3h | **Done** | 
| **UI-02** | Simple Command Parser (`help`, `clear`, `echo`, `version`, `setcolor`, `reboot`, `panic`) | Highest | 6h | **Done** |
| **MEM-01** | Physical Memory Manager (PMM) - Bitmap | Highest | 10h | **Done** | 
| **MEM-02** | Virtual Memory Manager (Paging) | High | 12h | **Done** |
| **MEM-04** | Higher-Half Kernel Mapping | Medium | 8h | **Done** |
| **MEM-03** | Kernel Heap Allocator (kmalloc) | Medium | 8h | **Done** |
| **CORE-05** | Task State Segment (TSS) Setup | High | 6h | **Done** |
| **CORE-04** | PIT (Programmable Interval Timer) Setup | Highest | 4h | **Done** |
| **DRV-03** | Circular Buffers for Data I/O | Medium | 3h | **Todo** |
| **MEM-05** | Slab Allocator for Kernel Objects | Low | 12h | **Todo** |
| **PROC-01** | Process Control Block (PCB) & TCB Structures | High | 10h | **Done** |
| **PROC-02** | Context Switching Logic | High | 12h | **Done** |
| **SCHED-01** | Basic Round-Robin CPU Scheduler | High | 10h | **Done** |
| **SYS-01** | System Call Dispatcher (int 0x80) | High | 8h | **In Progress** |
| **SEC-01** | Ring 3 (User Mode) & Memory Protection | Medium | 15h | **Todo** |
| **FS-01** | Initial RAMDisk (initrd) Support | Medium | 10h | **Todo** |
| **FS-02** | Virtual File System (VFS) Abstraction | Medium | 15h | **Todo** |
| **STOR-01** | ATA/IDE Storage Driver | Medium | 12h | **Todo** |
| **FS-03** | Basic FAT or Ext2 Filesystem Driver | Low | 20h | **Todo** |
| **CONC-01** | Synchronization Primitives (Spinlocks/Mutex) | Medium | 8h | **Done** |
| **PROC-03** | Multithreading & Thread Management | Medium | 15h | **Todo** |
| **IPC-01** | Inter-Process Communication (Pipes/Signals) | Low | 15h | **Todo** |
| **SCHED-02** | Multi-level Feedback Queue (MLFQ) | Low | 15h | **Todo** |
| **MEM-06** | Swap Memory & Page-to-Disk Logic | Low | 20h | **Todo** |
| **CORE-05** | Deadlock Detection & Avoidance Logic | Low | 12h | **Todo** |
| **ARCH-01** | x86_64 Long Mode Transition | Low | 20h | **Todo** |

## Glossary of Modules
- **CORE**: Foundational CPU/Hardware initialization.
- **MEM**: Physical, Virtual, and Dynamic memory management.
- **PROC/SCHED**: Task management, threading, and CPU time slicing.
- **FS/STOR**: Filesystems and persistent storage I/O.
- **SEC/SYS**: System security, Ring privileges, and the User-Kernel bridge.

## Roadmap Phases

### Phase 1: Interactive Foundation (CURRENT)
Perfecting the interface between the human and the hardware. This includes the VGA driver, keyboard interrupts, and the shell.

### Phase 2: Memory & Resources
Moving beyond hardcoded memory. Implementing the PMM, VMM, and dynamic memory allocation to support complex data structures.

### Phase 3: Filesystem & User Mode (NEXT)
1. **User Mode Isolation (SEC-01)**: Utilize the finished TSS to drop privilege levels.
2. **System Calls (SYS-01)**: The bridge for user programs to access kernel resources.
3. **Initial RAMDisk (FS-01)**: Loading the first files into memory via GRUB.
4. **VFS Abstraction (FS-02)**: Creating a consistent file interface (`open`, `read`, `write`).

### Phase 4: Advanced Memory & Concurrency
1. **Slab Allocator (MEM-05)**: Optimizing allocation for small kernel objects (TCBs, VFS nodes).
2. **Synchronization (CONC-01)**: Implementing Spinlocks and Mutexes for thread-safe operations.