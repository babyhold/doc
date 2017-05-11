
#obj-m+=mod_a.o
obj-m:=mod1.o
mod1-y:=mod_a.o

KERNELDIR :=/home/sunming/RTL_TMP/RTL8871_SDK/linux-2.6.30/

PWD:=$(shell pwd)

all:
	make -C $(KERNELDIR) M=$(PWD) modules
	#make -C $(KDIR) M=$(PWD) modules
modules_install:
    $(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install
clean:
	rm -rf .*.cmd *.o *.mod.c *.ko .tmp_versions *.order *symvers