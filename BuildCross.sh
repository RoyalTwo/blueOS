#! /usr/bin/bash

DOWNLOAD_DIR="compiler"

echo "Building GCC Cross Compiler"
echo "Note: Currently only supports apt package manager!"
if [ "$(id -u)" -ne 0 ]; then
        echo 'FATAL: This script must be run by root! Exiting...' >&2
        exit 1
fi

if ! command -v apt --version &> /dev/null
then
    echo "FATAL: Apt not found! Exiting..."
    exit 1
fi
if [ -d $DOWNLOAD_DIR ]; then
        rm -rf $DOWNLOAD_DIR/
fi
mkdir compiler 2>/dev/null # 2>/dev/null hides error from stdout

# Install dependencies
apt install -y build-essential
apt install -y bison
apt install -y flex
apt install -y libgmp3-dev
apt install -y libmpc-dev
apt install -y libmpfr-dev
apt install -y texinfo
apt install -y libisl-dev
apt install -y curl
apt install -y xz-utils

# Download binutils and gcc
curl https://ftp.gnu.org/gnu/binutils/binutils-2.42.tar.xz -o $DOWNLOAD_DIR/binutils.tar.xz
curl https://ftp.gnu.org/gnu/gcc/gcc-14.1.0/gcc-14.1.0.tar.xz -o $DOWNLOAD_DIR/gcc.tar.xz
cd $DOWNLOAD_DIR/

# Unzip
tar xf binutils.tar.xz
tar xf gcc.tar.xz

# Prep
cwd=$(pwd)
PREFIX=$cwd
TARGET=x86_64-elf
export PATH="$PREFIX/bin:$PATH"
chmod -R 777 $cwd

# Binutils
mkdir build-binutils
cd build-binutils
../binutils-2.42/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror || exit 1
make -j 8 || exit 1
make -j 8 install || exit 1

# GCC
cd ../
which -- $TARGET-as || echo "$TARGET-as is not in the PATH"

mkdir build-gcc
cd build-gcc
../gcc-14.1.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers || exit 1
make -j 8 all-gcc || exit 1
make -j 8 all-target-libgcc || exit 1
make -j 8 install-gcc || exit 1
make -j 8 install-target-libgcc || exit 1

# Finish
cd ../
echo "Compiler built! Use $DOWNLOAD_DIR/bin/$TARGET-elf-gcc!"
