INCLUDE_PATH=../vsd_driver/
include ../config.mk

vsd_userspace: main.cpp $(INCLUDE_PATH)/vsd_ioctl.h
	g++ $< -I$(INCLUDE_PATH) -Wall -Wextra -Werror -std=c++11 -o $@
	cp $@ $(VM_SHARE_PATH)

.PHONY: clean

clean:
	rm -rf vsd_userspace
