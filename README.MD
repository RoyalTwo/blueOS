# blueOS
#### A custom 32-bit operating system built from scratch for the x86 architecture.
Right now, the kernel is being loaded with GRUB in 32-bit protected mode. I've written a custom bootloader in assembly with disk loading and conversion from LBA to CHS, but opted to switch to GRUB afterwards since I built it for learning and wanted to focus primarily on the kernel. You can still find the custom bootloader in archive/, it also uses a FAT12 file system.

Currently, there is a VGA driver and terminal driver implemented with a **super** basic shell. The kernel loads these and prints some things to the screen, along with loading a Global Descriptor Table and Interrupt Descriptor Table.

Future tasks:
- [x] printf implementation
- [x] Interrupt Descriptor Table
- [ ] Keyboard Input
- [ ] File system (FAT32)
- [ ] Paging
- [ ] Memory management
## Why?
I started this project to learn about how operating systems work, low level concepts, and low level programming. Since starting, I have learned so much about assembly, processor architecture, memory, etc. and it's been exceedingly helpful to becoming a better and more experienced programmer. Plus, it's very satisfying to watch your own code boot! 😊
## Building/Running
To run the OS:
- Use the .ISO file in /bin/ with QEMU-i386 or Rufus to burn onto a USB, then boot on real hardware

To build from source:
- You need a cross compiler. I'm working on a script that will build one for you. Once you have one, simply run "make run-iso" to run the ISO or "make run" to run just the kernel.
