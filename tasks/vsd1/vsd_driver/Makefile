MOD_NAME := vsd_driver
ccflags-y := -Wall -Wextra -Wno-unused-parameter -Werror

ifneq ($(KERNELRELEASE),)
	obj-m := $(MOD_NAME).o
	header-y += vsd_ioctl.h
	$(MOD_NAME)-objs := module.o
else
	include ../config.mk
all:
	$(MAKE) -C $(KDIR) M=$$PWD
	cp $(MOD_NAME).ko $(VM_SHARE_PATH)

clean:
	$(MAKE) -C $(KDIR) M=$$PWD clean
endif
