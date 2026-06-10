#!/bin/sh

# Set the target architecture
export HOST=i686-elf

# Setup the toolchain paths
export COMPILER_PATH="/home/kevin/Documents/Codes/ProjectsOnline/KevOSCompiler/CrossCompiler/opt/cross"
export PATH="$COMPILER_PATH/bin:$PATH"

export PROJECT_DIR="/home/kevin/Documents/Codes/ProjectsOnline/KevOS"
export SYSROOT="$PROJECT_DIR/sysroot"

# Diagnostic: Check if the compiler binary actually exists and is executable
if [ ! -f "$COMPILER_PATH/bin/$HOST-gcc" ]; then
    echo "Warning: $HOST-gcc not found at $COMPILER_PATH/bin/$HOST-gcc"
    echo "Please check if the path is correct or if the compiler was installed successfully."
elif [ ! -x "$COMPILER_PATH/bin/$HOST-gcc" ]; then
    echo "Warning: $HOST-gcc found but is not executable. Try: chmod +x $COMPILER_PATH/bin/*"
else
    echo "Cross-compiler confirmed: $($HOST-gcc --version | head -n 1)"

    # Diagnostic for internal compiler frontends (cc1 and cc1plus)
    GCC_VERSION=$($HOST-gcc -dumpversion)
    CC1_PATH="$COMPILER_PATH/libexec/gcc/$HOST/$GCC_VERSION/cc1"
    CC1PLUS_PATH="$COMPILER_PATH/libexec/gcc/$HOST/$GCC_VERSION/cc1plus"

    if [ ! -f "$CC1_PATH" ]; then
        echo "Error: cc1 not found at $CC1_PATH."
    elif [ ! -x "$CC1_PATH" ]; then
        echo "Error: cc1 found but is not executable. Please run: chmod +x \"$CC1_PATH\""
    fi

    if [ ! -f "$CC1PLUS_PATH" ]; then
        echo "Error: cc1plus not found at $CC1PLUS_PATH."
    elif [ ! -x "$CC1PLUS_PATH" ]; then
        echo "Error: cc1plus found but is not executable. Please run: chmod +x \"$CC1PLUS_PATH\""
    else
        # Check internal binutils (as, ld) which GCC calls internally
        INTERNAL_BIN="$COMPILER_PATH/$HOST/bin"
        if [ -d "$INTERNAL_BIN" ]; then
            if [ ! -x "$INTERNAL_BIN/as" ] || [ ! -x "$INTERNAL_BIN/ld" ]; then
                echo "Warning: Internal binutils in $INTERNAL_BIN are not executable. GCC may fall back to host tools!"
            fi
        fi
    fi
fi