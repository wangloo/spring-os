
ifeq ($(TARGET_APP_CC),)
  CC = musl-gcc
else
  CC = $(TARGET_APP_CC)
endif


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


LINK_LIBS = $(addprefix -l, $(APP_LINK_LIBS))
__LIBS_DEPS = $(addprefix $(TARGET_LIBS_DIR)/lib, $(APP_LINK_LIBS))
LIBS_DEPS = $(addsuffix .a, $(__LIBS_DEPS))
LIBS_DEPS += $(TARGET_LIBS_DIR)/libc.a

LDFLAGS := --static -L$(TARGET_LIBS_DIR) $(LINK_LIBS)
LDFLAGS += $(APP_LDFLAGS)

CFLAGS := -Wall -g -D_XOPEN_SOURCE -D_GNU_SOURCE \
	-Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing \
	-fno-common -Werror-implicit-function-declaration -O$(O_LEVEL) \
	-Wno-format-security -I$(TARGET_INCLUDE_DIR) -I$(UAPI_INC_DIR)

CFLAGS	+= $(APP_CFLAGS) -DAPP_TAG=\"$(DBG_TAG)\" 
CFLAGS  += -MD -MP

ifeq ($(ARCH),aarch64)
  CFLAGS += -march=armv8-a
endif

ifneq ($(TEXT_START),)
  CFLAGS += -Wl,-Ttext=$(TEXT_START)
endif

src_c	:= $(SRC_C)
src_s	:= $(SRC_S)

OBJS	:= $(src_c:%.c=%.o)
OBJS	+= $(src_s:%.S=%.o)

OBJS_D	= $(OBJS:%.o=%.d)

$(TARGET) : $(OBJS) $(LIBS_DEPS)
	$(PROGRESS)
	$(CC) $^ -o $@ $(LDFLAGS) $(CFLAGS)
	$(QUIET) echo "Build $(TARGET) Done ..."

%.o : %.c
	$(PROGRESS)
	$(QUIET) $(CC) $(CFLAGS) -c $< -o $@

%.o : %.S
	$(PROGRESS)
	$(QUIET) $(CC) $(CFLAGS) -D__ASSEMBLY__ -c $< -o $@

.PHONY: clean distclean install

$(TARGET_OUT_DIR)/$(APP_INSTALL_DIR)/%: %
	$(TARGET_INSTALL) -D -m 755 $< $@
#	$(STRIP) -s $@

install: $(TARGET_OUT_DIR)/$(APP_INSTALL_DIR)/$(TARGET)

clean:
	$(QUIET) rm -rf $(TARGET) $(OBJS) $(LDS) $(OBJS_D)

distclean: clean
	rm -rf cscope.in.out cscope.out cscope.po.out tags

-include $(OBJS_D)
