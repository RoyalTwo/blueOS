FROM ubuntu:24.04

RUN apt-get update && apt upgrade -y
RUN apt-get install nasm build-essential -y

COPY compiler /usr/local/bin/compiler/

RUN /usr/local/bin/compiler/bin/x86_64-elf-gcc --version
