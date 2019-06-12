#!/bin/bash

if [ $# -ne 1 ];then
    echo "usage: ./$0 channel"
    exit 1
fi
CHANNEL=$1

DIR=/sys/class/pwm/pwmchip0

# activate the PWM. On H3 only 1 PWM is supported, so exporting PWM 0
if [ ! -d ${DIR}/pwm${CHANNEL} ]; then
    echo 0 > ${DIR}/export
    if [ $? -ne 0 ]; then
        echo "unsupported channel ${CHANNEL}"
        exit 1
    fi
fi

# set period to 10ms
echo 10000000 > ${DIR}/pwm${CHANNEL}/period                  # 100Hz

#echo "inversed" > ${DIR}/pwm${CHANNEL}/polarity
echo "normal" > ${DIR}/pwm${CHANNEL}/polarity

# enable the PWM
echo 1 > ${DIR}/pwm${CHANNEL}/enable

# set duty cycle to 1ms
echo 5000000 > ${DIR}/pwm${CHANNEL}/duty_cycle

echo "pwm outputting"
