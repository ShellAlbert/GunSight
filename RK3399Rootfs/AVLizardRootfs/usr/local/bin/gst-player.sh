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

# ----------------------------------------------------------
# base setup

file="/home/pi/demo.mp4"
adev="0"
vsnk="kmssink"

#----------------------------------------------------------
usage()
{
	echo "Usage: $0 [ARGS]"
	echo
	echo "Options:"
	echo "  -h                 show this help message and exit"
	echo "  -f <media file>    *"
	echo "  -J, --jack"
	echo "  -H, --hdmi"
	echo
	exit 1
}

parse_args()
{
	TEMP=`getopt -o "f:JHxh" --long "jack,hdmi" -n "$SELF" -- "$@"`
	if [ $? != 0 ] ; then exit 1; fi
	eval set -- "$TEMP"

	while true; do
		case "$1" in
			-f ) file=$2; shift 2;;
			-J|--jack) adev="0"; shift 1;;
			-H|--hdmi) adev="1"; shift 1;;
			-x ) vsnk="rkximagesink"; shift 1;;

			-h ) usage; exit 1 ;;
			-- ) shift; break ;;
			*  ) echo "invalid option $1"; usage; return 1 ;;
		esac
	done
}

#----------------------------------------------------------
SELF=$0
parse_args $@

if [ -f ${file} ]; then
	echo "Playing ${file} [card${adev}] ..."
else
	echo "Error: ${file}: No such file"
	exit -1
fi

#----------------------------------------------------------
export DISPLAY=:0.0

GSTC="gst-launch-1.0 uridecodebin uri=file://${file} name=decoder \
		decoder. ! queue ! ${vsnk} \
		decoder. ! queue ! audioconvert ! alsasink device=\"hw:${adev}\""

if [ $vsnk = "kmssink" -o "$(id -un)" = "pi" ]; then
	eval "${GSTC}"
else
	su pi -c "${GSTC}"
fi

