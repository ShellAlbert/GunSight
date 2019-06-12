#!/bin/bash

SYS_EDP=/sys/class/drm/card0-eDP-1
SYS_HDMI=/sys/class/drm/card0-HDMI-A-1

if grep "lcd=" /proc/cmdline >/dev/null ; then
   exit 0
else
   echo "off" > ${SYS_EDP}/status
   echo "eDP `cat ${SYS_EDP}/status`"
fi
