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


_all: all

PHONY += kernel

all: kernel


kernel:
	$(Q)echo "\n\033[32m ---> Build Kernel ... \033[0m \n"
	$(Q)$(MAKE) $(MFLAGS) -C ukern
# @$(MAKE) $(MFLAGS) -C kernel install



.PHONY: clean
clean:
	@$(MAKE) $(MFLAGS) -C ukern clean

