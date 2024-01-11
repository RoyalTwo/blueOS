# gladOS
#### A custom 32-bit operating system built from scratch for the x86 architecture.
Right now, the kernel is being loaded with GRUB in 32-bit protected mode. I've written a custom bootloader in assembly with disk loading and conversion from LBA to CHS, but opted to switch to GRUB afterwards since I built it for learning and wanted to focus primarily on the kernel. You can still find the custom bootloader in archive/, it also uses a FAT12 file system.

Currently working on the basics of the kernel including interrupts, a VGA driver, etc.
## Why?
I started this project to learn about how operating systems work, low level concepts, and low level programming. Since starting, I have learned so much about assembly, processor architecture, memory, etc. and it's been exceedingly helpful to becoming a better and more experienced programmer. Plus, it's very satisfying to watch your own code boot! 😊

## Building
So far, this has only been tested on Ubuntu running on Windows Subsystem for Linux 2.

**Requirements**: QEMU, qemu-system-i386, grub-pc-bin, xorriso

Once you have the requirements, simply run either:
- make run - This will run the kernel only
- make run-iso - This converts the kernel to an ISO and runs the full ISO, as if it were a real machine