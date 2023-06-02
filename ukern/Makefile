NAME = spring
all:

ARCH		    ?= aarch64
CROSS_COMPILE 	?= aarch64-none-linux-gnu-

OBJ_DIR ?= ./build
SRC_DIR ?= .
INC_DIR ?= . \
		   ./inc
BIN_DIR ?= ./build

BINARY = $(BIN_DIR)/$(NAME)
SRCS := $(shell find $(SRC_DIRS) -name '*.cpp' -or -name '*.c' -or -name '*.S')
OBJS := $(SRCS:$(SRC_DIR)/%=$(OBJ_DIR)/%.o) 
INCS = $(addprefix -I, $(INC_DIR))

SPRING_LDS = $(OBJ_DIR)/spring.lds

# Make variables (CC, etc...)
AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)gcc

CFLAGS  +=  -Wall -g -O2   \
			--static -nostdlib -fno-builtin \
			-Wno-unused-function
LDFLAGS += -T $(SPRING_LDS)


all: $(BINARY)

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

$(SPRING_LDS):$(SRC_DIR)/spring.lds
	@$(CC) -E -x c -P  $(INCS) -o $@ $<





.PHONY: clean gdb run
run: $(BINARY)
	bash ./tools/run-qemu.sh
gdb: $(BINARY)
	bash ./tools/gdb.sh
clean:
	-rm -rf $(BINARY) $(OBJ_DIR) 
