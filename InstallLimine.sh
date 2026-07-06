#! /usr/bin/env bash
# Download the latest Limine binary release for the 7.x branch.
git clone https://github.com/limine-bootloader/limine.git --branch=v11.x-binary --depth=1

# Build "limine" utility.
make -C limine
