#!/bin/bash

# Copyright (C) Guangzhou FriendlyARM Computer Tech. Co., Ltd.
# (http://www.friendlyarm.com)
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, you can access it online at
# http://www.gnu.org/licenses/gpl-2.0.html.

SELF=$0

#----------------------------------------------------------
# local functions

function usage()
{
	echo "Usage: $0 [ARGS]"
	echo ""
	echo "Options:"
	echo "  -h                   show this help message and exit"
	echo "  -f <config file>     default: /etc/X11/xorg.conf"
	echo "  -m <mode>            default: off"
	echo "  -r                   restart lightdm"
	echo ""
	echo "Supported mode:"
	echo "  CW    clockwise, 90 degrees"
	echo "  UD    upside down, 180 degrees"
	echo "  CCW   counter clockwise, 270 degrees"
	echo "  off   NO rotation"
	echo ""
	echo "Copyright (C) FriendlyARM <http://www.friendlyarm.com>"
}

function parse_args()
{
	TEMP=`getopt -o "f:m:rh" -n "$SELF" -- "$@"`
	if [ $? != 0 ] ; then exit 1; fi
	eval set -- "$TEMP"

	while true; do
		case "$1" in
			-f ) x11conf=$2; shift 2;;
			-m ) rotation=$2; shift 2;;
			-r ) restartX=true; shift 1;;
			-h ) usage; exit 1 ;;
			-- ) shift; break ;;
			*  ) echo "invalid option $1"; usage; return 1 ;;
		esac
	done

	case ${rotation} in
	CW | right)
		rotation="right"
		swap_xy="true"; invert_X="false"; invert_Y="true" ;;
	CCW | left)
		rotation="left"
		swap_xy="true"; invert_X="true"; invert_Y="false" ;;
	UD | inverted)
		rotation="inverted"
		swap_xy="false"; invert_X="true"; invert_Y="true" ;;
	*)
		rotation="normal"
		swap_xy="false"; invert_X="false"; invert_Y="false" ;;
	esac
}

function generate_head()
{
	cat > $1 <<- EOF
# /etc/X11/xorg.conf (xorg X Window System server configuration file)
# 
# Copyright (C) Guangzhou FriendlyARM Computer Tech. Co., Ltd.
# (http://www.friendlyarm.com)
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, you can access it online at
# http://www.gnu.org/licenses/gpl-2.0.html.

	EOF
}

function generate_conf()
{
	cat >> $1 <<- EOF
Section "Monitor"
    Identifier          "eDP-1"
    Option              "Rotate" "${rotation}"
EndSection

Section "Screen"
    Identifier          "Screen0"
    Monitor             "eDP-1"
EndSection

Section "ServerLayout"
    Identifier          "DefaultLayout"
    Screen              "Screen0"
EndSection

Section "InputClass"
    Identifier          "touchscreen"
    MatchIsTouchscreen  "on"
    MatchDevicePath     "/dev/input/event*"
    Driver              "evdev"
    Option              "SwapAxes" "${swap_xy}"
    Option              "InvertX" "${invert_X}"
    Option              "InvertY" "${invert_Y}"
EndSection
	EOF
}

#----------------------------------------------------------

parse_args $@

HWARCH=`uname -m`
if [ "x$HWARCH" = "xaarch64" -o "x${HWARCH:0:3}" = "xarm" ]; then
	true ${x11conf:=/etc/X11/xorg.conf}
else
	true ${x11conf:=xorg.conf.new}
	restartX=false
fi

if [ -f ${x11conf} ]; then
	echo "Backup current config: ${x11conf}~"
	mv -f ${x11conf} ${x11conf}~
fi

echo -n "Generating ${x11conf}..."
generate_head		${x11conf}
generate_conf		${x11conf}
echo "done."

if [ "${restartX}" = "true" ]; then
	echo -n "Re-starting lightdm.service..."
	systemctl restart lightdm
	echo "done."
fi

