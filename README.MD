# blueOS

![GitHub Actions Build Status](https://github.com/RoyalTwo/blueOS/actions/workflows/build-release.yml/badge.svg)
![Release Version](https://img.shields.io/github/v/release/royaltwo/blueOS?color=green)

#### A custom 64-bit operating system built from scratch for the x86_64 architecture.

The kernel is currently being loaded with Limine. I've written a custom bootloader in assembly with disk loading and conversion from LBA to CHS, but opted to switch to Limine afterwards since I built it for learning and wanted to focus primarily on the kernel. You can still find the custom bootloader in archive/, it also uses a FAT12 file system.

The linker script, makefile, and entry point are provided as part of the Limine standard example. Everything else (the whole kernel) is custom.

## Running

- Download the [latest ISO](https://github.com/RoyalTwo/blueOS/releases/latest) from the Releases tab (automatically built with every tag)
- Use `qemu-system-x86_64` to run in a virtual machine or flash to USB with Rufus

### Goals

#### General

- [ ] Kernel/Usermode separation
- [ ] Basic shell
- [ ] GUI

#### CPU

- [x] Global Descriptor Table
- [x] Interrupt Descriptor Table

#### Drivers

- [x] Serial
- [ ] PS/2 Keyboard
- [ ] File system (FAT32)

#### Memory

- [ ] Paging
- [ ] Memory management

## Why?

I started this project to learn about how operating systems work, low level concepts, and low level programming. Since starting, I have learned so much about assembly, processor architecture, memory, etc. and it's been exceedingly helpful to becoming a better and more experienced programmer. Plus, it's very satisfying to watch your own code boot! 😊

## Building/Running

To build from source:

1. Run `BuildCross.sh` to build a cross-compiler for the project (only supports APT package manager for now)
2. Run `make` to build executable
3. (Optional) Run `./InstallLimine.sh` to install limine to the project directory (only relevant for ISO)
4. (Optional) Create the ISO with `./CreateISO.sh`
5. The executable and iso are in bin/

To run the OS:

- Use the executable file (bin/blueOS) or ISO (bin/blueOS.iso) with qemu-system-x86_64 to emulate or Rufus to burn onto a USB, then boot on real hardware
- Run `make run` (uses qemu-system-x86_64 under the hood)
    - Requires running `./InstallLimine.sh` beforehand!
