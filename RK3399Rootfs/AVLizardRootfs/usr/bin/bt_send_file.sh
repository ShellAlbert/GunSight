#!/bin/bash

# apt-get install bluetooth bluez obexftp openobex-apps python-gobject ussp-push time bc
# dd if=/dev/urandom of=test.dat count=1 bs=1M

if [ $# -ne 4 ]; then
    echo "Usage:`basename $0` [-a address] [-f file]"
    exit 1
fi

function reset_bt()
{
    OPR=${1}
    DEV=${2}
    BTWAKE=/proc/bluetooth/sleep/btwake
    if [ -f ${BTWAKE} ]; then
        echo 0 > ${BTWAKE}
    fi
    index=`rfkill list | grep ${DEV} | cut -f 1 -d":"` 
    if [[ -n ${index}} ]]; then
        rfkill ${OPR} ${index}
        sleep 1
    fi
}

#hcitool scan
reset_bt unblock sunxi-bt
reset_bt unblock hci0
hciconfig hci0 up
mac_address=50:C8:E5:A7:31:D2
while getopts "a:f:" opt
do
    case $opt in
        a )
            mac_address=$OPTARG;;              
        f )
            file=$OPTARG
            file_size=`du -k ${file} | cut -f 1`;;
        ? )
            echo "Usage:`basename $0` [-a address] [-f file]"                    
            exit 1;;
        esac
done

channel=`sdptool browse ${mac_address} | grep -A 7 "Service Name: OBEX Object Push" | grep "Channel"`
channel=${channel#*: }

time_log=/tmp/bt_time.log
/usr/bin/time -p -f "%e" -o ${time_log} ussp-push ${mac_address}@${channel} ${file} ${file}
if [ $? -eq 0 ]; then
    real_time=`cat ${time_log}`
    speed=`echo "scale=1;${file_size}/${real_time}" | bc`
    echo "send ${file_size}K finish, speed=${speed} K/s"
else
    echo "fail to send file"
fi
