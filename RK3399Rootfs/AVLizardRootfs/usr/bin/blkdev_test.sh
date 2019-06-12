#!/bin/bash

if [ $# -ne 2 ]; then
    echo "Usage:`basename $0` [-s size]"
    exit 1
fi

file1=test1.dat
file2=test2.dat
size=64
while getopts "s:" opt
do
    case $opt in
        s )
            size=$OPTARG;;              
        ? )
            echo "Usage:`basename $0` [-s size]"
            exit 1;;
        esac
done

echo "generating ${size}M file..."
dd if=/dev/urandom of=${file1} count=${size} bs=1M
sync
cp -v ${file1} ${file2}
sync

md5_1=`md5sum ${file1} | cut -b 1-32`
md5_2=`md5sum ${file2} | cut -b 1-32`

if [ "${md5_1}" = "${md5_2}" ]; then
    echo "${md5_1} = ${md5_2}, blkdev test ok"
else
    echo "${md5_1} != ${md5_2}, blkdev test fail"
fi

rm -rf ${file1} ${file2}
