TARGET 		= sh.elf
APP_CFLAGS 	= -I.
# APP_LINK_LIBS	= fdt

SRC_C	:= $(wildcard *.c)


APP_INSTALL_DIR = ramdisk

TEXT_START := 0x8000000

include $(projtree)/scripts/app_build.mk
