PWD		:= $(shell pwd)

ifeq ($(VERBOSE),1)
  QUIET =
else
  QUIET = @
endif

ifeq ($(QUIET),@)
PROGRESS = @echo Compiling $@ ...
endif


ifeq ($(TARGET),)
  $(error "target is not defined")
endif

ifeq ($(APP_TAG),)
  APP_TAG = $(TARGET)
endif

DBG_TAG = $(basename $(APP_TAG))

CFLAGS := -Wall -D_XOPEN_SOURCE -D_GNU_SOURCE \
	-Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing \
	-fno-common -Werror-implicit-function-declaration -O$(O_LEVEL) \
	-Wno-format-security -I$(TARGET_INCLUDE_DIR) -I$(UAPI_INCLUDE_DIR)

CFLAGS	+= $(LIB_CFLAGS)
CFLAGS  += -MD -MP
CFLAGS	+= -DAPP_TAG=\"$(DBG_TAG)\"

ifeq ($(BUILD_DEBUG),1)
  CFLAGS += -g
endif

ifeq ($(ARCH),aarch64)
  CFLAGS += -march=armv8-a
endif

src_c	:= $(SRC_C)
src_s	:= $(SRC_S)

OBJS	:= $(src_c:%.c=%.o)
OBJS	+= $(src_s:%.S=%.o)

OBJS_D	= $(OBJS:%.o=%.d)

$(TARGET) : $(OBJS)
	$(PROGRESS)
	$(QUIET) $(AR) crv $@ $^
	$(QUIET) echo "Build $(TARGET) Done ..."

%.o : %.c
	$(PROGRESS)
	$(QUIET) $(CC) $(CFLAGS) -c $< -o $@

%.o : %.S
	$(PROGRESS)
	$(QUIET) $(CC) $(CFLAGS) -D__ASSEMBLY__ -c $< -o $@

.PHONY: clean distclean install

$(TARGET_INCLUDE_DIR)/%: $(PWD)/include/%
	$(TARGET_INSTALL) -D -m 644 $< $@

$(TARGET_LIBS_DIR)/%: %
	$(TARGET_INSTALL) -D -m 755 $< $@

install-headers: $(INSTALL_HEADERS:include/%=$(TARGET_INCLUDE_DIR)/%)
install-libs: $(TARGET_LIBS_DIR)/$(TARGET)

install: install-headers install-libs

clean:
	$(QUIET) rm -rf $(TARGET) $(OBJS) $(LDS) $(OBJS_D)

distclean: clean
	rm -rf cscope.in.out cscope.out cscope.po.out tags

-include $(OBJS_D)
