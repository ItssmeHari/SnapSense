# Makefile — SnapSense Linux Kernel Module
#
# Usage:
#   make          → build snapsense.ko
#   make clean    → remove build files
#   sudo insmod snapsense.ko   → load driver
#   sudo rmmod snapsense       → unload driver
#   dmesg | tail -20           → see kernel log messages
#   cat /dev/snapsense         → read temperature

obj-m += snapsense.o

# Path to running kernel's build system
KDIR := /lib/modules/$(shell uname -r)/build

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean
