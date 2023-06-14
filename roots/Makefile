TARGET 		= roots.elf
APP_CFLAGS 	= 
# APP_LINK_LIBS	= fdt

SRC_C	:= $(wildcard src/*.c)

APP_INCLUDE_DIR = ./inc
APP_INSTALL_DIR = ramdisk

TEXT_START := 0x8000000

include $(projtree)/scripts/app_build.mk
