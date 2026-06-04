#!/bin/sh
set -e
. ./config.sh

echo "Cleaning previous builds..."
rm -rf sysroot
rm -rf bin
mkdir -p bin

echo "Installing headers..."
./headers.sh

COMMON_FLAGS="--sysroot=$SYSROOT -I. -Ikernel/include -isystem=/usr/include -ffreestanding -march=i686 -O2 -Wall -Wextra -fno-exceptions -fno-rtti"

echo "Building Libc..."
$HOST-g++ $COMMON_FLAGS -c libc/stdio/putchar.cpp -o bin/putchar.o
$HOST-g++ $COMMON_FLAGS -c libc/string/memmove.cpp -o bin/memmove.o
$HOST-g++ $COMMON_FLAGS -c libc/stdlib/abort.cpp -o bin/abort.o
$HOST-g++ $COMMON_FLAGS -c libc/stdlib/raise.cpp -o bin/raise.o
$HOST-g++ $COMMON_FLAGS -c libc/stdio/printf.cpp -o bin/printf.o
$HOST-g++ $COMMON_FLAGS -c libc/string/memset.cpp -o bin/memset.o
$HOST-g++ $COMMON_FLAGS -c libc/string/strlen.cpp -o bin/strlen.o
$HOST-g++ $COMMON_FLAGS -c libc/string/memcpy.cpp -o bin/memcpy.o

echo "Building Kernel..."
# Compile Assembly components (arch-specific)
$HOST-g++ -c kernel/arch/i386/boot/boot.S -o bin/boot.o -ffreestanding -O2 -Wall -Wextra
$HOST-g++ -c kernel/arch/i386/boot/crti.s -o bin/crti.o -ffreestanding
$HOST-g++ -c kernel/arch/i386/boot/crtn.s -o bin/crtn.o -ffreestanding
$HOST-g++ -c kernel/arch/i386/cpu/interrupts.S -o bin/interrupts.o -ffreestanding -O2 -Wall -Wextra

# Compile Driver/Arch components
$HOST-g++ $COMMON_FLAGS -c kernel/arch/i386/drivers/tty.cpp -o bin/tty.o
$HOST-g++ $COMMON_FLAGS -c kernel/arch/i386/cpu/io.cpp -o bin/io.o
$HOST-g++ $COMMON_FLAGS -c kernel/arch/i386/cpu/pic.cpp -o bin/pic.o
$HOST-g++ $COMMON_FLAGS -c kernel/arch/i386/cpu/gdt.cpp -o bin/gdt.o
$HOST-g++ $COMMON_FLAGS -c kernel/arch/i386/cpu/idt.cpp -o bin/idt.o
$HOST-g++ $COMMON_FLAGS -c kernel/arch/i386/cpu/isr.cpp -o bin/isr.o
$HOST-g++ $COMMON_FLAGS -c kernel/arch/i386/cpu/irq.cpp -o bin/irq.o
$HOST-g++ $COMMON_FLAGS -c kernel/arch/i386/drivers/keyboard.cpp -o bin/keyboard.o

# Compile C++ Kernel (main logic)
$HOST-g++ $COMMON_FLAGS -c kernel/kernel/kernel.cpp -o bin/kernel.o

# Link the final kernel
$HOST-g++ -T kernel/arch/i386/boot/linker.ld -o bin/kevos.kernel -ffreestanding -O2 -nostdlib \
    bin/crti.o bin/boot.o bin/interrupts.o bin/kernel.o bin/tty.o bin/io.o bin/pic.o bin/gdt.o bin/idt.o bin/isr.o bin/irq.o bin/keyboard.o bin/putchar.o bin/printf.o bin/strlen.o bin/memcpy.o bin/memmove.o bin/memset.o bin/abort.o bin/raise.o bin/crtn.o -lgcc

if [ -f bin/kevos.kernel ]; then
    echo "Build successful: bin/kevos.kernel"
else
    echo "Build failed!"
    exit 1
fi