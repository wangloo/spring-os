PHONY := _all
_all:

ifeq ($(VERBOSE),1)
	Q =
else
	Q = @
endif

MAKE  		= make
MFLAGS		:= --no-print-directory

PHONY += all
_all: all

PHONY += kernel

all: kernel


kernel:
	$(Q) echo "\n\033[32m ---> Build Kernel ... \033[0m \n"
	$(Q) $(MAKE) $(MFLAGS) -C kernel
	$(Q) $(MAKE) $(MFLAGS) -C kernel install
