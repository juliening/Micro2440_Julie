

.PHONY: all clean

obj-m += mini2440_led_platform_device.o mini2440_led_platform_driver.o

KDIR := /home/wallace/workSPACE/source_lib/linux_2.6.32.2_mini2440_20100106

all: 
	make ARCH=arm CROSS_COMPILE=arm-linux- -C $(KDIR) SUBDIRS=$(PWD) modules #-I $(KDIR)/arch/arm/mach-s3c2410/include/mach #$(KDIR)/include/linux

clean:
	make ARCH=arm CROSS_COMPILE=arm-linux- -C $(KDIR) SUBDIRS=$(PWD) clean
