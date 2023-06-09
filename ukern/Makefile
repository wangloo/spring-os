NAME = spring
all:

ARCH		    ?= aarch64
CROSS_COMPILE 	?= aarch64-none-linux-gnu-

ROOT_DIR ?= .
OBJ_DIR ?= ./build
SRC_DIR ?= ./src
INC_DIR ?= . \
		   ./inc
BIN_DIR ?= ./build

BINARY = $(BIN_DIR)/$(NAME)
SRCS := $(shell find $(SRC_DIR) -name '*.cpp' -or -name '*.c' -or -name '*.S')
OBJS := $(SRCS:$(SRC_DIR)/%=$(OBJ_DIR)/%.o) 
INCS = $(addprefix -I, $(INC_DIR))

offset_h  := $(ROOT_DIR)/asm/asm_offset.h
offset_s  := $(OBJ_DIR)/src/asm_offset.S
offset_c  := $(ROOT_DIR)/tools/asm_offset.c


SPRING_LDS = $(OBJ_DIR)/spring.lds

# Make variables (CC, etc...)
AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)gcc

CFLAGS  +=  -Wall -g -O1   \
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
	@$(CC) $(CFLAGS) $(INCS) -c -o $@ $<
$(OBJ_DIR)/%.S.o: $(SRC_DIR)/%.S
	@echo "CC      $<"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -D__ASSEMBLY__ $(INCS) -c -o $@ $<

$(SPRING_LDS):$(ROOT_DIR)/spring.lds
	@$(CC) -E -x c -P  $(INCS) -o $@ $<

$(offset_s): $(offset_c)
	@echo "CC      $(offset_s)"
	@mkdir -p $(dir $@)
	@gcc $(INCS) -S $< -o $@

$(offset_h): $(offset_s)
	@ $(call cmd_offsets)






.PHONY: clean gdb run
run: $(BINARY)
	bash ./tools/run-qemu.sh
gdb: $(BINARY)
	bash ./tools/gdb.sh
clean:
	-rm -rf $(BINARY) $(OBJ_DIR) $(offset_h)
