#!/bin/bash
make ARCH=arm64 nanopi4_linux_defconfig
export PATH=/opt/FriendlyARM/toolchain/6.4-aarch64/bin:$PATH
make menuconfig
make ARCH=arm64 nanopi4-images
