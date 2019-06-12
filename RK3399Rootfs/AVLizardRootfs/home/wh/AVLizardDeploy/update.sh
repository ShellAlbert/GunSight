#!/bin/bash

# update the wifi hotspot name & password.
if [ $# != 2 ]; then
	echo "Error parameter input!"
	echo "Usage: sudo ./update.sh HotspotName  Password"
	exit -1
fi
HotspotName=$1
Password=$2
rm -rf .hotspotname .password
echo $HotspotName > .hotspotname
echo $Password > .password
if [ ! -f .hotspotname -o ! -f .password ] ; then 
	echo "Error update failed!"	
	exit -1
fi
echo "Success,new hotspot will generate after reboot!"
exit 0
