#!/bin/sh
set -e
. ./config.sh

mkdir -p isodir/boot/grub
cp bin/kevos.kernel isodir/boot/kevos.kernel
cat << EOF > isodir/boot/grub/grub.cfg
menuentry "KevOS" {
	multiboot /boot/kevos.kernel
}
EOF

grub-mkrescue -o kevos.iso isodir

echo "Starting QEMU..."
qemu-system-i386 -cdrom kevos.iso -serial stdio