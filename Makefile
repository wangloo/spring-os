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
STRIP   = $(CROSS_COMPILE)strip
export AS LD CC

projtree := $(shell pwd)
srctree  := .
export projtree


UAPI_INC_DIR = $(projtree)/generic/include/
export UAPI_INC_DIR

OUT_DIR = $(projtree)/out

_all: all

PHONY += kernel roots libc prepare

all: ramdisk kernel roots 

objdirs:
	$(Q) mkdir -p $(srctree)/out
	$(Q) mkdir -p $(srctree)/out/include
	$(Q) mkdir -p $(srctree)/out/lib
	$(Q) mkdir -p $(srctree)/out/ramdisk
	$(Q) mkdir -p $(srctree)/out/rootfs/bin
	$(Q) mkdir -p $(srctree)/out/rootfs/sbin
	$(Q) mkdir -p $(srctree)/out/rootfs/driver
	$(Q) mkdir -p $(srctree)/out/rootfs/etc

roots: libc
	$(Q) echo "\n\033[32m ---> Compiling Root Service ... \033[0m \n";	
	$(Q)$(MAKE) $(MFLAGS) -C roots

ramdisk: kernel roots
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

libc:
	$(Q) echo "\n\033[32m---> Build LIBC ... \033[0m \n"
	$(Q) $(MAKE) $(MFLAGS) -C libc -j 16
	$(Q) $(MAKE) $(MFLAGS) -C libc install

prepare: objdirs
	$(Q) cd libc; ./build.sh $(OUT_DIR) $(ARCH) $(CROSS_COMPILE)
#	$(Q) cd kernel; make $(TARGET_PLATFORM)_defconfig
	cp -f ukern/inc/uapi/* libc/include/ukern/

run: ramdisk kernel
	bash ./tools/run-qemu.sh
gdb: ramdisk kernel
	bash ./tools/gdb.sh

.PHONY: clean $(PHONY)
clean: clean-libc
	@$(MAKE) $(MFLAGS) -C ukern clean
	@$(MAKE) $(MFLAGS) -C tools/mkrmd clean
	rm -f ramdisk.bin

clean-libc:
	$(Q) echo "\033[32m Clean libc \033[0m"
	$(Q) $(MAKE) $(MFLAGS) -C libc clean