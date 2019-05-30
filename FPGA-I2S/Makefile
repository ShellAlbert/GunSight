obj-m += fpga_i2s.o
build:
	make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -C ../kernel-rockchip M=$(shell pwd) modules
#	aarch64-linux-gnu-gcc test.c  -o test64.bin
#	arm-linux-gcc cap_play.c  -o cap_play.bin -I/home/zhangshaoyan/armbuild/libalsa4arm/include -L/home/zhangshaoyan/armbuild/libalsa4arm/lib -lasound
.PHONY clean:
clean:	
	rm -rvf *.mod.* *.o modules.* Module.* *.ko 
	rm -rvf test64.bin cap_play.bin
