#/bin/bash

#author:ShellAlbert.
#date:July 19,2018.
#this script is used to check the wifi hotspot by periodly.
#if the hotspot is not exist then restart it.
#

#export libraries.
LIB_QT=$PWD/libqt
LIB_OPENCV=$PWD/libopencv
LIB_ALSA=$PWD/libalsa
LIB_OPUS=$PWD/libopus
LIB_RNNOISE=$PWD/librnnoise
LIB_X264=$PWD/libx264
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$LIB_QT:$LIB_OPENCV:$LIB_ALSA:$LIB_OPUS:$LIB_RNNOISE:$LIB_X264

#get cpuid to act as wifi hotspot name.
CPUID=`cat /proc/cpuinfo | grep Serial | awk '{print $3}'`

#start function.
function startHotspot()
{
	nmcli connection add type wifi ifname '*' con-name AVLizard autoconnect no ssid "AVLizard(${CPUID})"
	nmcli connection modify AVLizard 802-11-wireless.mode ap 802-11-wireless.band bg ipv4.method shared
	nmcli connection modify AVLizard 802-11-wireless-security.key-mgmt wpa-psk 802-11-wireless-security.psk 12345678
	nmcli connection up AVLizard
	return 0
}

#stop function.
function stopHotspot()
{
	nmcli connection delete AVLizard
	return 0
}

#the main entry.
#make sure we run this with root priority.
if [ `whoami` != "root" ];then
	echo "<error>: latch me with root priority please."
	exit -1
fi

#start wifi hotspot.
stopHotspot
startHotspot

#check if 3 usb uvc cameras are plugged into system.
#video0: default.
#video1/video2: camera1.
#video3/video4: camera2.
#video5/video6: camera3.
CAM_NUM=`ls -l /dev/video* | wc -l`
if [ $CAM_NUM -lt 7 ];then
	echo "<error>: I need 3 cameras at least."
	exit -1
fi
#remove other video nodes that we donot care.
rm /dev/video0
rm /dev/video2
rm /dev/video4
rm /dev/video6

#make sure we have 3 cameras at least.
CAM_NUM=`ls -l /dev/video* | wc -l`
if [ $CAM_NUM -lt 3 ];then
	echo "<error>: I need 3 cameras at least."
	exit -1
fi

#the main loop.
while true
do
	#check wifi hotspot.
	LIVESTR=`nmcli connection show -active | grep AVLizard`
	if [ -z "$LIVESTR" ];then
		echo "<error>:cannot detected AVLizard hotspot,restart it."
		stopHotspot
		sleep 5
		startHotspot
	else
		echo "<okay>:wifi hotspot okay."
	fi


	#check the video app.
	if [ -f "/tmp/AVLizard.pid" ];then
		PID=`cat /tmp/AVLizard.pid`
		kill -0 $PID
		if [ $? -eq 0 ];then
			echo "<okay>:lizard pid detect okay."
		else
			echo "<error>:lizard pid detect error! schedule it."
			./AVLizard.bin  &
		fi
	else
		echo "<error>:lizard pid file not exist! schedule it."
		./AVLizard.bin  &
	fi

	#check periodly every 5 seconds.
	sleep 30
done

#the end of file.
