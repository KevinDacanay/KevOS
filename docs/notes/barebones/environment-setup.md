# Building an i686 Cross-Compiler for Operating System Development

Before creating an operating system, you first need a **cross-compiler toolchain**.

A cross-compiler allows you to compile programs for another platform — in this case:

```text id="w5cm66"
Host System  → Your Linux Machine
Target System → i686-elf Operating System
```

This prevents your OS kernel from accidentally linking against your host operating system libraries.

---

# Required Dependencies

Building GCC and Binutils requires several tools and libraries.

## Required Packages

* GCC
* G++
* Make
* Bison
* Flex
* GMP
* MPFR
* MPC
* Texinfo
* ISL (optional)

---

# Installing Dependencies (Ubuntu / Debian)

```bash id="3m7m9i"
sudo apt update
sudo apt install -y \
    build-essential \
    bison \
    flex \
    libgmp3-dev \
    libmpc-dev \
    libmpfr-dev \
    texinfo \
    libisl-dev \
    wget \
    curl
```

---

# Setting Up Environment Variables

These variables define:

* Where the compiler installs
* The target architecture
* PATH updates

```bash id="yz9nbi"
export PREFIX="$HOME/opt/cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"
```

---

# Making Environment Variables Persistent

```bash id="5d2qpm"
echo 'export PREFIX="$HOME/opt/cross"' >> ~/.bashrc
echo 'export TARGET=i686-elf' >> ~/.bashrc
echo 'export PATH="$PREFIX/bin:$PATH"' >> ~/.bashrc

source ~/.bashrc
```

---

# Creating a Source Directory

```bash id="hmx8yz"
mkdir -p $HOME/src
cd $HOME/src
```

---

# Downloading Binutils

Binutils provides:

* Assembler (`as`)
* Linker (`ld`)
* Binary tools

---

## Download & Extract

```bash id="r4t3x6"
wget https://ftp.gnu.org/gnu/binutils/binutils-2.41.tar.gz
tar -xvzf binutils-2.41.tar.gz
```

---

# Building Binutils

```bash id="v14o6g"
mkdir -p $HOME/src/build-binutils
cd $HOME/src/build-binutils
```

---

## Configure Binutils

```bash id="a1g2qx"
../binutils-2.41/configure \
    --target=$TARGET \
    --prefix="$PREFIX" \
    --with-sysroot \
    --disable-nls \
    --disable-werror
```

---

## Compile & Install

```bash id="mf1lt6"
make -j$(nproc)
make install
```

---

# Verifying the Assembler

```bash id="l1s4c4"
which -- $TARGET-as || echo "$TARGET-as is not in the PATH"
```

Expected output:

```text id="xqjz1v"
/home/youruser/opt/cross/bin/i686-elf-as
```

---

# Downloading GCC

```bash id="g4q8tk"
wget https://ftp.gnu.org/gnu/gcc/gcc-13.2.0/gcc-13.2.0.tar.gz
tar -xvzf gcc-13.2.0.tar.gz
```

---

# Building GCC

```bash id="z2t5k7"
mkdir -p $HOME/src/build-gcc
cd $HOME/src/build-gcc
```

---

## Configure GCC

```bash id="k9r6dp"
../gcc-13.2.0/configure \
    --target=$TARGET \
    --prefix="$PREFIX" \
    --disable-nls \
    --enable-languages=c,c++ \
    --without-headers \
    --disable-hosted-libstdcxx
```

---

## Compile GCC

```bash id="v7h4u9"
make all-gcc -j$(nproc)
make all-target-libgcc -j$(nproc)
make all-target-libstdc++-v3 -j$(nproc)
```

---

## Install GCC

```bash id="j3y8ew"
make install-gcc
make install-target-libgcc
make install-target-libstdc++-v3
```

---

# Verifying the Compiler

```bash id="v8b2rz"
which -- $TARGET-gcc || echo "$TARGET-gcc is not in the PATH"

$TARGET-gcc --version
```

Expected result:

```text id="p5b6zh"
i686-elf-gcc (GCC) 13.2.0
```

---

# Understanding the Result

You now have a **bare-metal cross-compiler**.

This compiler:

* Can compile kernel code
* Does NOT include a standard C library
* Cannot create normal Linux executables
* Is designed for operating system development

Installed location:

```text id="9gq9ah"
$HOME/opt/cross/bin/i686-elf-gcc
```

---

# Using the Compiler

Compile code using:

```bash id="9i4ow8"
i686-elf-gcc
```

Or directly:

```bash id="g9h3mt"
$HOME/opt/cross/bin/i686-elf-gcc
```

---

# Why Cross-Compilers Matter

Using your host compiler directly is dangerous because:

* It links against host libraries
* Assumes Linux or Windows runtime behavior
* Produces incompatible binaries

A cross-compiler ensures:

* Full control
* Bare-metal compatibility
* No host OS dependencies

---

# Troubleshooting

---

## `ld: cannot find -lgcc`

This usually means:

* `libgcc` was not built
* `libgcc` was not installed properly

Rebuild and reinstall:

```bash id="y6x5e7"
make all-target-libgcc
make install-target-libgcc
```

---

## Manual `libgcc` Location

```text id="6z7r1v"
$PREFIX/lib/gcc/$TARGET/<gcc-version>/libgcc.a
```

---

# Cleaning Failed Builds

If the build becomes corrupted:

```bash id="f2d5s1"
make distclean
```

Then restart the build process.

---

# Binutils Version Confusion

Do NOT accidentally download old versions such as:

```text id="fgv5it"
Binutils 2.9
```

Use modern versions like:

* 2.41
* 2.42
* newer

Older versions may fail with modern GCC.

---

# Missing System Header Directories

Some targets require missing directories to exist.

Create them manually:

```bash id="q3v9uw"
mkdir -p $SYSROOT/mingw/include
mkdir -p $SYSROOT/mingw/lib
```

---

# GCC `libsanitizer` Errors

If GCC fails building `libsanitizer`, add:

```bash id="v8s2ty"
--disable-libsanitizer
```

to the GCC configure command.

---

# Recommended OS Development Order

When starting your operating system, build components in dependency order.

---

# 1. Screen Output

Implement:

* Printing strings
* Decimal integers
* Hexadecimal integers

This becomes your primary debugging system.

Example:

```c id="u8f4h9"
kprint("Kernel loaded!");
```

---

# 2. Serial Port Output

Serial debugging is extremely useful because:

* No scrolling issues
* Easy logging
* Automated testing support

Recommended early.

---

# 3. Interrupt & Exception Handling

Implement:

* ISR handlers
* Register dumps
* Fault reporting

This dramatically improves debugging.

---

# 4. Plan Your Memory Map

Decide:

* Kernel virtual address
* Heap location
* Stack layout
* User memory layout

Good planning prevents major rewrites later.

---

# 5. Heap Allocation

Implement:

* `malloc()`
* `free()`

Dynamic memory becomes essential quickly.

---

# Suggested Development Philosophy

Use a:

```text id="0r8l4m"
Least-Dependent-First
```

approach.

Build lower-level systems before higher-level systems.

---

# Example Dependency Logic

A GUI might depend on:

* Filesystem
* Bitmap loading
* Resource management

But the filesystem itself does NOT require a GUI.

So build:

1. Filesystem interface
2. Filesystem implementation
3. GUI later

---

# Recommended Early Priorities

```text id="8i7wd2"
1. Bootloader
2. Screen Output
3. Serial Debugging
4. Interrupts
5. Memory Management
6. Heap
7. Keyboard Input
8. Shell
9. Filesystem
10. Multitasking
```

---

# Final Notes

Once your cross-compiler works, you are ready to begin:

```text id="u6n7fr"
Bootloader → Kernel → VGA Output → Interrupts → Shell
```

That is the beginning of a real operating system.
