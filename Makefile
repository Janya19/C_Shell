CC     := gcc
CFLAGS := -std=c99 -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE=700 \
          -Wall -Wextra -Werror -Wno-unused-parameter -fno-asm -Iinclude \
          -MMD -MP

SRC_DIR := src
OBJ_DIR := build
INC_DIR := include
TARGET  := shell.out

# Collect all .c files in src/
SRC := $(wildcard $(SRC_DIR)/*.c)
OBJ := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC))
DEP := $(OBJ:.o=.d)   # dependency files

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Compile .c → .o while also generating .d dependency files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

-include $(DEP)   # include auto-generated .d files

.PHONY: all clean
