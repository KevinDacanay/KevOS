#!/bin/sh
set -e
. ./config.sh

mkdir -p isodir/boot/grub
cp bin/myos.kernel isodir/boot/myos.kernel
cat << EOF > isodir/boot/grub/grub.cfg
menuentry "myos" {
	multiboot /boot/myos.kernel
}
EOF

grub-mkrescue -o myos.iso isodir

echo "Starting QEMU..."
qemu-system-i386 -cdrom myos.iso -serial stdio