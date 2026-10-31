#!/bin/bash
# build.sh - builds the whole OS image from scratch
set -e

mkdir -p build

CC=${CC:-gcc}
OBJCOPY=${OBJCOPY:-objcopy}

echo "[1/5] Assembling bootloader..."
nasm -f bin boot/boot.asm -o build/boot.bin

echo "[2/5] Assembling kernel entry stub..."
nasm -f elf32 kernel/kernel_entry.asm -o build/kernel_entry.o

echo "[3/5] Compiling C sources..."
CFLAGS="-m32 -ffreestanding -fno-pie -fno-stack-protector -mno-mmx -mno-sse -mno-sse2 -Wall -Wextra"
$CC $CFLAGS -c kernel/kernel.c   -o build/kernel.o
$CC $CFLAGS -c kernel/vga.c      -o build/vga.o
$CC $CFLAGS -c kernel/keyboard.c -o build/keyboard.o
$CC $CFLAGS -c kernel/shell.c    -o build/shell.o

echo "[4/5] Linking kernel..."
$CC -m32 -no-pie -T linker.ld -o build/kernel.elf -ffreestanding -nostdlib \
    build/kernel_entry.o \
    build/kernel.o build/vga.o build/keyboard.o build/shell.o

echo "[5/5] Converting to flat binary + building disk image..."
$OBJCOPY -O binary build/kernel.elf build/kernel.bin
cat build/boot.bin build/kernel.bin > build/os-image.bin

echo ""
echo "Done. Kernel size: $(wc -c < build/kernel.bin) bytes"
echo "Run with: qemu-system-x86_64 -drive format=raw,file=build/os-image.bin"
