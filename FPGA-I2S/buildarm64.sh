#!/bin/bash
#for ARMv8
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- firefly_linux_defconfig
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- rk3399-firefly-linux.img -j4

