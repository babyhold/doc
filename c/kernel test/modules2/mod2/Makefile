
obj-m:=mod2.o
mod2-y:=mod_b.o

KBUILD_EXTRA_SYMBOLS=/home/sunming/RTL_TMP/modules2/mod1/Module.symvers
KERNELDIR :=/home/sunming/RTL_TMP/RTL8871_SDK/linux-2.6.30/

PWD:=$(shell pwd)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

modules_install:
    $(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install
clean:
	rm -rf *.o *~core .depend .*.cmd *.ko *.ko.* *.mod.c .tmp_versions *odule* $(TARGET)