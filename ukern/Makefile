NAME = spring
all:


ROOT_DIR ?= .
OBJ_DIR ?= ./build
SRC_DIR ?= ./src
INC_DIR ?= . \
		   ./inc \
		   $(UAPI_INC_DIR)
BIN_DIR ?= ./build

BINARY = $(BIN_DIR)/$(NAME)
SRCS := $(shell find $(SRC_DIR) -name '*.c' -or -name '*.S')
OBJS := $(SRCS:$(SRC_DIR)/%=$(OBJ_DIR)/%.o) 
INCS := $(addprefix -I, $(INC_DIR))
DEPS := $(OBJS:.o=.d)

offset_h  := $(ROOT_DIR)/asm/asm_offset.h
offset_s  := $(OBJ_DIR)/asm_offset.S
offset_c  := $(ROOT_DIR)/tools/asm_offset.c

# 临时的，保留进入ukern编译的合法性
# 如果以后修改为都在外面编译运行，以下部分代码可以删除
ARCH		  = aarch64
PLATFORM	= qemu
CROSS_COMPILE 	= aarch64-none-linux-gnu-


# Make variables (CC, etc...)
AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)gcc



SPRING_LDS = $(OBJ_DIR)/spring.lds



CFLAGS += -Wall -g -O1   \
			 		-MMD -MP      \
			 		--static -nostdlib -fno-builtin \
					-Wno-unused-function \
					-ffixed-x18
					
LDFLAGS += -T $(SPRING_LDS)

-include kern_macro.mk



all: $(offset_h) $(BINARY)

$(BINARY): $(OBJS) $(SPRING_LDS)
	@echo "LD      $@"
	@$(LD) $(LDFLAGS) -o $@ $(OBJS) 

$(OBJ_DIR)/%.c.o: $(SRC_DIR)/%.c
	@echo "CC      $<"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -D__KERNEL__ $(INCS) -c -o $@ $<
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


.PHONY: clean

clean:
	-rm -rf $(BINARY) $(OBJ_DIR) $(offset_h)
