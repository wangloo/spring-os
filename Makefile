_all:


ifeq ($(VERBOSE),1)
  Q =
else
  Q = @
endif

MAKE  		= make
MFLAGS		:= --no-print-directory
export MAKE MFLAGS

ARCH		  ?= aarch64
PLATFORM	?= qemu
CROSS_COMPILE 	?= aarch64-none-linux-gnu-
export ARCH PLATFORM CROSS_COMPILE

# Make variables (CC, etc...)
AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)gcc
export AS LD CC

projtree := $(shell pwd)
export projtree


UAPI_INC_DIR = $(projtree)/generic/include/
export UAPI_INC_DIR

_all: all

PHONY += kernel

all: ramdisk kernel

ramdisk: kernel
	$(Q)$(MAKE) $(MFLAGS) -C tools/mkrmd
	$(Q) echo "\n\033[32m ---> Packing Ramdisk image ... \033[0m \n"
	$(Q) qemu-img create -f raw ramdisk.bin 64M
	$(Q) chmod +x ./tools/mkrmd/mkrmd
	$(Q) ./tools/mkrmd/mkrmd -f ramdisk.bin tiny.elf
#	$(Q) qemu-img -q resize ramdisk.bin 64M  2> /dev/null

kernel:
	$(Q)echo "\n\033[32m ---> Build Kernel ... \033[0m \n"
	$(Q)$(MAKE) $(MFLAGS) -C ukern
# @$(MAKE) $(MFLAGS) -C kernel install

run: ramdisk kernel
	bash ./tools/run-qemu.sh
gdb: ramdisk kernel
	bash ./tools/gdb.sh

.PHONY: clean
clean:
	@$(MAKE) $(MFLAGS) -C ukern clean
	@$(MAKE) $(MFLAGS) -C tools/mkrmd clean
	rm -f ramdisk.bin

