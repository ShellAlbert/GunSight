#!/bin/bash

if [ $# -ne 1 ]; then
    echo "${0} [libmali file name]"
    exit 1
fi

CopyFileFast() {
    if [ -f $2 ]; then
        # files are different
        cmp --silent $1 $2 || cp -f $1 $2
    else
        cp -f $1 $2
    fi
}

LIBMALI=$1
cd /usr/rk3399-libs
[ -d /usr/rk3399-libs/lib64 ] && CopyFileFast lib64/${LIBMALI} /usr/rk3399-libs/lib64/libMali.so
[ -d /usr/lib/aarch64-linux-gnu ] && CopyFileFast lib64/${LIBMALI} /usr/lib/aarch64-linux-gnu/libMali.so
# [ -d /usr/rk3399-libs/lib32 ] && CopyFileFast lib32/${LIBMALI} /usr/rk3399-libs/lib32/libMali.so
# [ -d /usr/lib/arm-linux-gnueabihf ] && CopyFileFast lib32/${LIBMALI} /usr/lib/arm-linux-gnueabihf/libMali.so
