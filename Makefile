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
OBJDUMP = $(CROSS_COMPILE)objdump
READELF = $(CROSS_COMPILE)readelf
export AS LD CC STRIP OBJDUMP READELF

projtree := $(shell pwd)
srctree  := .
export projtree

# Make variables (CC, etc...)
TARGET_AS	= $(TARGET_CROSS_COMPILE)as
TARGET_LD	= $(TARGET_CROSS_COMPILE)ld
TARGET_CC	= $(TARGET_CROSS_COMPILE)gcc
TARGET_APP_CC	= $(projtree)/out/bin/musl-gcc
TARGET_CPP	= $(TARGET_CC) -E
TARGET_AR	= $(TARGET_CROSS_COMPILE)ar
TARGET_NM	= $(TARGET_CROSS_COMPILE)nm
TARGET_STRIP	= $(TARGET_CROSS_COMPILE)strip
TARGET_OBJCOPY	= $(TARGET_CROSS_COMPILE)objcopy
TARGET_OBJDUMP	= $(TARGET_CROSS_COMPILE)objdump
TARGET_INSTALL = $(projtree)/tools/install.sh

TARGET_INCLUDE_DIR = $(projtree)/out/include
TARGET_LIBS_DIR = $(projtree)/out/lib
TARGET_OUT_DIR = $(projtree)/out
UAPI_INC_DIR = $(projtree)/generic/include/
export TARGET_AS TARGET_LD TARGET_CC TARGET_APP_CC TARGET_CPP TARGET_AR TARGET_NM TARGET_STRIP TARGET_INSTALL
export TARGET_INCLUDE_DIR TARGET_LIBS_DIR TARGET_OUT_DIR
export UAPI_INC_DIR

# Path definition
OUT_DIR = $(projtree)/out
SERVS_DIR = service

SERVS_SUB_DIR = $(foreach dir, $(SERVS_DIR), $(shell find $(dir) -maxdepth 1 -type d))
SERVS_TARGETS = $(filter-out $(SERVS_DIR),$(SERVS_SUB_DIR))

_all: all

PHONY += kernel servs libc prepare

all: ramdisk kernel servs dump

objdirs:
	$(Q) mkdir -p $(srctree)/out
	$(Q) mkdir -p $(srctree)/out/include
	$(Q) mkdir -p $(srctree)/out/lib
	$(Q) mkdir -p $(srctree)/out/ramdisk
	$(Q) mkdir -p $(srctree)/out/rootfs/bin
	$(Q) mkdir -p $(srctree)/out/rootfs/sbin
	$(Q) mkdir -p $(srctree)/out/rootfs/driver
	$(Q) mkdir -p $(srctree)/out/rootfs/etc

servs: libc
	$(Q) set -e;					\
	for i in $(SERVS_TARGETS); do 			\
		if [ -f $$i/Makefile ]; then		\
			echo "\n\033[32m ---> Compiling App $$i ... \033[0m \n";	\
			$(MAKE) $(MFLAGS) -C $$i ;		\
			$(MAKE) $(MFLAGS) -C $$i install;	\
		fi					\
	done
roots: libc
	$(Q) echo "\n\033[32m ---> Compiling Root Service ... \033[0m \n";	
	$(Q)$(MAKE) $(MFLAGS) -C roots
	$(Q)$(MAKE) $(MFLAGS) -C roots install
dump: kernel servs
	$(Q)$(OBJDUMP) --dwarf=info out/ramdisk/spring.elf > out/spring_dwarf_info.dump
	$(Q)$(OBJDUMP) --dwarf=frames out/ramdisk/spring.elf > out/spring_dwarf_frames.dump
	$(Q)$(OBJDUMP) --dwarf=frames-interp out/ramdisk/spring.elf > out/spring_dwarf_frames_interp.dump
	$(Q)$(OBJDUMP) --dwarf=line out/ramdisk/spring.elf > out/spring_dwarf_lines.dump
	$(Q)$(OBJDUMP) -S out/ramdisk/spring.elf > out/spring.dump
	$(Q)$(OBJDUMP) -S out/ramdisk/roots.elf > out/roots.dump
ramdisk: kernel servs
	$(Q)$(MAKE) $(MFLAGS) -C tools/mkrmd 
	$(Q) echo "\n\033[32m ---> Packing Ramdisk image ... \033[0m \n"
	$(Q) qemu-img create -f raw out/ramdisk.bin 64M  > /dev/null
	$(Q) chmod +x ./tools/mkrmd/mkrmd 
	$(Q) ./tools/mkrmd/mkrmd -d out/ramdisk.bin out/ramdisk
#	$(Q) qemu-img -q resize ramdisk.bin 64M  2> /dev/null

kernel:
	$(Q)echo "\n\033[32m ---> Build Kernel ... \033[0m \n"
# Thirdparty don't need to compile every time
# But this can avoid forgetting now, delete it later
	$(Q)$(MAKE) $(MFLAGS) -C ukern thirdparty
	$(Q)$(MAKE) $(MFLAGS) -C ukern
	$(Q)$(MAKE) $(MFLAGS) -C ukern install
kernel-ut:
	$(Q)echo "\n\033[32m ---> Build Kernel(Ut) ... \033[0m \n"
	$(Q)$(MAKE) $(MFLAGS) -C ukern thirdparty
	$(Q)$(MAKE) $(MFLAGS) -C ukern UNITTEST=1
	$(Q)$(MAKE) $(MFLAGS) -C ukern install
	

libc:
	$(Q) echo "\n\033[32m---> Build LIBC ... \033[0m \n"
	$(Q) $(MAKE) $(MFLAGS) -C libc -j 16
	$(Q) $(MAKE) $(MFLAGS) -C libc install

prepare: objdirs
	$(Q) cd libc; ./build.sh $(OUT_DIR) $(ARCH) $(CROSS_COMPILE)
#	$(Q) cd kernel; make $(TARGET_PLATFORM)_defconfig
	cp -f generic/include/uapi/* libc/include/minos/

run: ramdisk kernel dump
	$(Q) bash ./tools/run-qemu.sh
run-gdb: ramdisk kernel
	$(Q) bash ./tools/run-qemu.sh -S
gdb-client: ramdisk kernel
	$(Q) bash ./tools/gdb.sh

.PHONY: clean $(PHONY)
clean: clean-libc clean-servs clean-ukern

	@$(MAKE) $(MFLAGS) -C tools/mkrmd clean

clean-ukern:
	$(Q) echo "\033[32m Clean ukern \033[0m"
	$(Q) $(MAKE) $(MFLAGS) -C ukern clean
clean-servs:
	$(Q)set -e;					\
	for i in $(SERVS_TARGETS); do 			\
		if [ -f $$i/Makefile ]; then		\
			echo "\033[32m Clean $$i \033[0m";		\
			$(MAKE) $(MFLAGS) -C $$i clean;	\
		fi					\
	done
clean-libc:
	$(Q) echo "\033[32m Clean libc \033[0m"
	$(Q) $(MAKE) $(MFLAGS) -C libc clean