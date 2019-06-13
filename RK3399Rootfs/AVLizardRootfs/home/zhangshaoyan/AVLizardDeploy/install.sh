#!/bin/bash

#unpack AVLizardDeploy-XX.XX.tar.bz2  to /home/pi/AVLizardDeploy

#sudo rm /bin/sh
#sudo ln -s /bin/bash /bin/sh

#config alsa
mkdir -p /home/zhangshaoyan/armbuild/libalsa4arm
tar jxvf share.tar.bz2  -C /home/zhangshaoyan/armbuild/libalsa4arm
echo "done"

#add startup scripts to boot script.
#add following to /etc/rc.local.
# cd /home/pi/AVLizardDeploy && ./monitor.sh &

