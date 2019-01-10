# this file describes how to burst OS (ubuntu based) to rk3399 emmc in linux.
1.Hold Recovery key and power on.
2.upgrade_tool ul MiniLoaderAll.bin
3.upgrade_tool di -p parameter.txt
4.upgrade_tool di uboot uboot.img
5.upgrade_tool di trust trust.img
6.upgrade_tool di resource resource.img
7.upgrade_tool di kernel kernel.img
8.upgrade_tool di boot boot.img
9.upgrade_tool di rootfs AVLizardRootfs.img
10.upgrade_tool RD
