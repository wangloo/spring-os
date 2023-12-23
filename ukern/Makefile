NAME = spring.elf
all:

Q = @ 



ROOT_DIR = .
OBJ_DIR = ./build
SRC_DIR = ./src
INC_DIR = . \
					./inc \
					$(UAPI_INC_DIR)
BIN_DIR = ./build
LIB_DIR = ./lib
INSTALL_DIR = ramdisk # install out

BINARY = $(BIN_DIR)/$(NAME)
SRCS := $(shell find $(SRC_DIR) -name '*.c' -or -name '*.S')
OBJS := $(SRCS:$(SRC_DIR)/%=$(OBJ_DIR)/%.o) 
INCS := $(addprefix -I, $(INC_DIR))
LIBS := $(shell find $(LIB_DIR) -name '*.a' -or -name '*.so')
DEPS := $(OBJS:.o=.d)

offset_h  := $(ROOT_DIR)/asm/asm_offset.h
offset_s  := $(OBJ_DIR)/asm_offset.S
offset_c  := $(ROOT_DIR)/tools/asm_offset.c

# 临时的，保留进入ukern编译的合法性
# 如果以后修改为都在外面编译运行，以下部分代码可以删除
ARCH		  = aarch64
PLATFORM	= qemu
CROSS_COMPILE 	= aarch64-none-linux-gnu-

MAKE  		= make
MFLAGS		:= --no-print-directory
export MAKE MFLAGS


# Make variables (CC, etc...)
AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)gcc



SPRING_LDS = $(OBJ_DIR)/spring.lds



CFLAGS += -Wall \
					-g \
					-O0   \
			 		-MMD -MP      \
			 		--static -nostdlib -fno-builtin \
					-Wno-unused-function \
					-fno-asynchronous-unwind-tables \
					-fno-unwind-tables \
					-fno-omit-frame-pointer \
					-ffixed-x18
# CFLAGS += -pg
					
LDFLAGS += -T $(SPRING_LDS) -L$(ROOT_DIR)/lib --start-group -lelf -ldwarf --end-group

ifeq ($(UNITTEST),1)
	CFLAGS += -DUNITTEST_ON
endif

-include kern_macro.mk

PHONY += install thirdparty

all: $(offset_h) $(BINARY)

install: $(BINARY)
	$(Q) cp $(BINARY) $(TARGET_OUT_DIR)/$(INSTALL_DIR)


thirdparty:
	$(Q) $(MAKE) $(MFLAGS) -C thirdparty

$(BINARY): $(OBJS) $(SPRING_LDS)
	@echo "LD      $@"
	$(Q) $(LD)  -o $@ $(OBJS) $(LDFLAGS)

# $(OBJS): thirdparty # 虽然thirdparty没用这种方式，保留以后扩展使用


$(OBJ_DIR)/%.c.o: $(SRC_DIR)/%.c
	@echo "CC      $<"
	@mkdir -p $(dir $@)
	$(Q) $(CC) $(CFLAGS) -D__KERNEL__ $(INCS) -c -o $@ $<
$(OBJ_DIR)/%.S.o: $(SRC_DIR)/%.S
	@echo "CC      $<"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -D__ASSEMBLY__ $(INCS) -c -o $@ $<

$(SPRING_LDS):$(ROOT_DIR)/spring.lds
	@$(CC) -E -x c -P  $(INCS) -o $@ $<

$(offset_s): $(offset_c)
	@echo "CC      $<"
	@mkdir -p $(dir $@)
	@gcc $(INCS) -S $< -o $@

$(offset_h): $(offset_s)
	@ $(call cmd_offsets)




# Include the .d makefiles. The - at the front suppresses the errors of missing
# Makefiles. Initially, all the .d files will be missing, and we don't want those
# errors to show up.
-include $(DEPS)


.PHONY: clean distclean $(PHONY)

clean:
	$(Q) $(MAKE) $(MFLAGS) -C thirdparty clean
	-rm -rf $(BINARY) $(OBJ_DIR) $(offset_h)

# lib and header from thirdparty
distclean: clean
	-rm -rf $(LIBS)
	-rm -rf $(shell find $(ROOT_DIR)/inc/lib -name "*.h")
