#!/bin/bash

if [ $# -eq 1 ]; then
    gpio=$1
else
    gpio="-1    -1 \
          12    -1 \
          11    -1 \
          203   198 \
          -1    199 \
          0     6 \
          2     -1 \
          3     200 \
          -1    201 \
          64    -1 \
          65    1 \
          66    67\
        "
fi
DIR=/sys/class/gpio
cd ${DIR}

# export 
for index in ${gpio}; do
    if [ ${index} -ge 0 ]; then
        echo ${index} > ${DIR}/export
    fi
done

sleep 1

# set direction
for index in ${gpio}; do
    if [ ${index} -ge 0 ]; then
        echo out > ${DIR}/gpio${index}/direction
    fi
done

# set high
echo high
for index in ${gpio}; do
    if [ ${index} -ge 0 ]; then
        echo 1 > ${DIR}/gpio${index}/value
    fi
done
sleep 1
echo low
for index in ${gpio}; do
    if [ ${index} -ge 0 ]; then
        echo 0 > ${DIR}/gpio${index}/value
    fi
done

# unexport 
for index in ${gpio}; do
    if [ ${index} -ge 0 ]; then
        echo ${index} > unexport
    fi
done