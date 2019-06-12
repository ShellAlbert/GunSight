#!/bin/bash

#----------------------------------------------------------
# mount fs

DATA_DIR=/tmp/media

if [ ! -d ${DATA_DIR} -a -b /dev/mmcblk0p1 ]; then
	mkdir -p ${DATA_DIR}
	mount /dev/mmcblk0p1 ${DATA_DIR}
fi

#----------------------------------------------------------
# get config

ROM_BASE=${DATA_DIR}/images
IMG_CONF=${ROM_BASE}/FriendlyARM.ini

if [ ! -f ${IMG_CONF} ]; then
	echo "Error: File ${IMG_CONF} not found, stop"
	exit -1
fi

function read_conf()
{
	local var="/^$1/ { print $2(\$2) }"
	echo `gawk -F= -e "${var}" ${IMG_CONF} | sed 's/\r//g'`
}

MEMTESTER=$(read_conf Tester-mem tolower)

#----------------------------------------------------------
# led helper

LED_BASE=/sys/class/leds

function led_set_gpio()
{
	echo gpio >  ${LED_BASE}/$1/trigger
	echo $2 >    ${LED_BASE}/$1/gpio
	echo $3 >    ${LED_BASE}/$1/inverted
}

function led_set_timer()
{
	echo timer > ${LED_BASE}/$1/trigger
	echo $2 >    ${LED_BASE}/$1/delay_off
	echo $3 >    ${LED_BASE}/$1/delay_on
}

function led_start_fading()
{
	if [ ! -d    ${LED_BASE}/ledp1 ]; then
		insmod /lib/modules/`uname -r`/leds-pwm.ko
	fi
	echo fading >${LED_BASE}/ledp1/trigger
}

function led_show_error()
{
	led_set_timer led2  80  50
	led_set_gpio  led1  43   1
}

#----------------------------------------------------------
# do testing

MEM_PROG=/usr/local/bin/mem
LOG_ODIR=${DATA_DIR}/log/memtester

RUN_COUNT=4
RUN_MEMSZ="100M"
RUN_SLEEP=60

if [ ! -f ${MEM_PROG} ]; then
	echo "Warn: ${MEM_PROG} not found, stop."
	exit -1
fi

if [[ ${MEMTESTER} != [yY]?(es) ]]; then
	exit 1
fi

mkdir -p ${LOG_ODIR}

echo -n "Starting Memory Testing..."
led_set_timer led1 200 150

for n in `seq 1 ${RUN_COUNT}`; do
	eval "${MEM_PROG} ${RUN_MEMSZ} > ${LOG_ODIR}/$n.log &"
	echo -n " $n"
done
echo "."

#----------------------------------------------------------
# do checking

while [ ${RUN_SLEEP} -gt 0 ]; do
	RUN_STATUS=`ps --no-headers -C mem | wc -l`

	if [[ ${RUN_STATUS} != ${RUN_COUNT} ]]; then
		echo "Error: Memory testing failed: ${RUN_COUNT} --> ${RUN_STATUS}"
		led_show_error
		sync
		exit -1
	else
		LOAD1=$(cut -d' ' -f1 /proc/loadavg)
		echo " `date +%T`  ${RUN_STATUS} running (${LOAD1})"
	fi

	let RUN_SLEEP-=5
	sleep 5
done

RUN_PIDS=`ps --no-headers -C mem -o pid`
kill ${RUN_PIDS}

led_start_fading
led_set_gpio  led1  78   1
sync

echo "Done. Log saved to sdcard: \\log\\memtester"

