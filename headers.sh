#!/bin/sh
set -e
. ./config.sh

mkdir -p "$SYSROOT/usr/include"

# Copy headers from libc and kernel
cp -R --preserve=timestamps libc/include/. "$SYSROOT/usr/include/."
cp -R --preserve=timestamps kernel/include/. "$SYSROOT/usr/include/."