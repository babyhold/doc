# Makefile2.6
ifneq ($(KERNELRELEASE),)
#kbuild syntax. dependency relationship of files and target modules are listed here.
obj-m +=hello.o
#obj-m :=gpio_driver.o
else
PWD :=$(shell pwd)
KEVER ?=$(shell uname -r)
#KDIR :=/home/wfg/linux-2.6.26.3/$(KEVER)/build
#KDIR :=/home/wfg/linux-2.6.26.3
KDIR := /home/sunming/828kernel/3.10.40
#KDIR :=/home/sunming/RTL_TMP/RTL8871_SDK/linux-2.6.30
default:
	echo hello
	echo $(KERNELRELEASE)
	echo $(obj-m)
	echo $(KEVER)
	echo $(PWD)
	#$(MAKE) -C $(KDIR) M=$(PWD)
	make -C $(KDIR) M=$(PWD) modules
clean:
	rm -rf .*.cmd *.o *.mod.c *.ko .tmp_versions *.order *symvers
endif
