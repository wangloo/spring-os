TARGET 		= roots.elf
APP_CFLAGS 	= -I.
# APP_LINK_LIBS	= fdt

SRC_C	:= $(wildcard *.c)

# APP_INCLUDE_DIR 一定要配置，否则在引用时会导致后面出错
APP_INCLUDE_DIR = ./inc
APP_INSTALL_DIR = ramdisk

TEXT_START := 0x8000000

include $(projtree)/scripts/app_build.mk
