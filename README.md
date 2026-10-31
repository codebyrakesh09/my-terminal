# MyOS

A tiny operating system built from scratch. It starts from a bootloader written in assembly, switches the computer into 32-bit mode, and then runs a small command-line terminal written in C.

You type commands, it answers. That's the whole idea.

## What can it do?

When it starts you get a prompt (`>`). These commands work:

| Command | What it does |
|---------|--------------|
| `help`  | Shows the list of commands |
| `about` | Prints a short description of the kernel |
| `clear` | Clears the screen |

Anything else prints `Unknown command`. Backspace works too.

## What you need

You need these installed on Linux:

- **NASM** - turns assembly code into machine code
- **i686-elf-gcc** (cross-compiler) - turns C code into machine code for a bare computer with no operating system. Your normal `gcc` will not work for this.
- **QEMU** - a fake computer that runs your OS in a window, so you don't need real hardware

Check the compiler is available:

```bash
which i686-elf-gcc
```

If that prints nothing, add this line to the end of `~/.bashrc`, then open a new terminal:

```bash
export PATH="$HOME/opt/cross/bin:$PATH"
```

## How to build and run

From the `my-os/` folder:

```bash
./build.sh
qemu-system-x86_64 -drive format=raw,file=build/os-image.bin
```

The first line builds everything. The second line starts the fake computer. Click inside the QEMU window so it can receive your typing.

If `build.sh` says "permission denied", run `chmod +x build.sh` once.

## Folder layout

```
my-os/
├── boot/
│   └── boot.asm           The bootloader (first code that runs)
├── kernel/
│   ├── kernel_entry.asm   Sets up a stack, then calls the C code
│   ├── kernel.c           The kernel's main function
│   ├── vga.c / vga.h      Writes text to the screen
│   ├── keyboard.c / .h    Reads keys from the keyboard
│   ├── shell.c / .h       The command prompt
│   └── io.h               Talks to hardware ports
├── linker.ld              Says where the kernel sits in memory
├── build.sh               Runs the whole build in one go
└── build/                 Made by build.sh (generated files go here)
```

## How it works, step by step

1. **You power on.** The computer starts in an old 16-bit mode (called Real Mode), even if it is a modern machine.
2. **The BIOS** (built-in startup program) reads the first 512 bytes of the disk and puts them in memory at address `0x7C00`. It only does this if the last two bytes are `0x55 0xAA`.
3. **The bootloader** (`boot.asm`) runs. It prints a message, then reads the kernel from the disk into memory at address `0x1000`.
4. **The bootloader switches to 32-bit mode** (called Protected Mode). To do that it turns on the A20 line, sets up a GDT (a small table that describes memory), and sets a bit in a special register called `CR0`.
5. **It jumps to the kernel.** `kernel_entry.asm` sets up a stack and calls `kmain()` in C.
6. **The kernel starts the shell.** `shell.c` prints `>`, waits for you to type a line, then runs the matching command.

### How text appears on screen

In 32-bit mode there are no BIOS helpers anymore. Instead, the screen is just a piece of memory at address `0xB8000`. Each character on screen uses 2 bytes: one for the letter, one for the color. Writing to that memory puts text on screen. This is what `vga.c` does.

### How typing works

The keyboard does not send letters. It sends numbers called *scancodes*. `keyboard.c` keeps asking the keyboard "is a key ready?" in a loop (this is called *polling*), reads the scancode, and turns it into a letter using a lookup table. Shift is tracked so capital letters and symbols work.

This version uses polling instead of interrupts because polling is much simpler and has fewer things that can go wrong.

<img width="793" height="475" alt="Screenshot 2026-10-31 113127" src="https://github.com/user-attachments/assets/7c246b6e-de28-4cdd-ad33-64ae0e5b7093" />


## Common problems

**`i686-elf-gcc: command not found`**
The cross-compiler is not on your PATH. See "What you need" above.

**The screen keeps flickering and the BIOS messages repeat**
The computer is crashing and restarting. The usual cause: the kernel is bigger than the number of sectors the bootloader reads. Check the kernel size:

```bash
wc -c build/kernel.bin
```

Divide that number by 512 and round up. In `boot/boot.asm`, make sure `KERNEL_SECTOR_COUNT` is at least that big. It is set to `10` now, which is enough for 5120 bytes. After changing it, run `./build.sh` again.

**`DISK READ ERROR!`**
The bootloader is trying to read more sectors than the disk image has. Lower `KERNEL_SECTOR_COUNT` so it matches the kernel size (see above).

## Notes

- Everything here is for learning. It has no memory protection, no multitasking, and no file system.
- The warning `LOAD segment with RWX permissions` from the linker is harmless for now.
