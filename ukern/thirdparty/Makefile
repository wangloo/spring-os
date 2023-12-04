_all:

INSTALL_INC_DIR=../inc/lib
INSTALL_LIB_DIR=../lib
LIBELF_DIR=$(shell realpath ./libelf)
LIBDWARF_DIR=$(shell realpath ./libdwarf/libdwarf)


_all: all

PHONY += libdwarf libelf install

all: libdwarf libelf install
	@echo "==> Thirdparty build OK"

libdwarf:
	make -C $(LIBDWARF_DIR)/build POSTINCS="--include=$(LIBDWARF_DIR)/spring.h" 

install: 
	cp $(LIBDWARF_DIR)/build/libdwarf.a $(INSTALL_LIB_DIR)
	cp $(LIBDWARF_DIR)/build/libdwarf.h $(INSTALL_INC_DIR)
	cp $(LIBDWARF_DIR)/libdwarf_ukern.h $(INSTALL_INC_DIR)
	cp $(LIBELF_DIR)/install/lib/libelf.a $(INSTALL_LIB_DIR)

.PHONY: clean $(PHONY)

clean:
	-rm -rf $(shell find $(INSTALL_LIB_DIR) -name "*.a")
	-rm -rf $(shell find $(INSTALL_INC_DIR) -name "*.h")
