INCLUDE_PATH=../vsd_driver/
CFLAGS=$(INCLUDE_PATH:%=-I%) -Wall -Wextra -Werror -g
TARGETS=vsdio vsd_test vsd_fork_test
include ../sub_config.mk

all: $(TARGETS)

$(TARGETS): %: %.o vsd_device.o
	gcc $^ -o $@
	cp $@ $(VM_SHARE_PATH)

$(TARGETS): vsd_device.h

%.o: %.c
	gcc $< -c $(CFLAGS) -o $@

.PHONY: clean

clean:
	rm -rf $(TARGETS) *.o >/dev/null 2>&1
