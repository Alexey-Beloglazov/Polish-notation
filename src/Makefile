SHELL := /bin/sh
CC      = gcc
CFLAGS  = -std=c11 -Wall -Wextra -Werror
LDFLAGS = -lm


SRC_DIR      := .
BUILD_DIR    := $(abspath ../build)


SRCS := graph.c tokenizer.c rpn.c stack.c
HDRS := token.h common.h stack.h
OBJS := $(addprefix $(BUILD_DIR)/,$(SRCS:.c=.o))
BIN  := $(BUILD_DIR)/graph

.PHONY: all clean
all: $(BIN)


$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)


$(BUILD_DIR)/%.o: %.c $(HDRS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@


$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)

clean:
	rm -rf $(BUILD_DIR)
