# gladOS
#### A custom 32-bit operating system built from scratch for the x86 architecture.
Right now, I have written a custom bootloader in assembly with debug messaging that can load more sectors of the disk image after converting from LBA to CHS. The image uses the FAT12 filesystem, although this will change later.

The next task is to write a GDT table and switch to Protected 32-bit mode, instead of Real 16-bit mode. After that, I will either rewrite the bootloader to be Multiboot compliant or I'll switch to GRUB; the bootloader was simply to learn about how OSes boot, and once in protected mode I'll have written most of the functionality for that. 

Then, I can start on the kernel in C and creating a standard library.
## Why?
I started this project to learn about how operating systems work, low level concepts, and low level programming. Since starting, I have learned so much about assembly, processor architecture, memory, etc. and it's been exceedingly helpful to becoming a better and more experienced programmer. Plus, it's very satisfying to watch your own code boot! 😊

## Building
So far, this has only been tested on Ubuntu running on Windows Subsystem for Linux 2.

**Requirements**: QEMU, qemu-system-i386, bochs, bochsbios, vgabios, bochs-sdl2, bochs-wx

Once you have the necessary programs, there are two files that will build the OS image and run it for you:
- Build_Run.sh - Builds the image and runs it through QEMU
- Build_Debug.sh - Builds the image and runs it through Bochs for debugging

Simply run either of the scripts and the operating system should boot in QEMU or Bochs.