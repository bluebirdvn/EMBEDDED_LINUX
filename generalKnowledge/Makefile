CC = gcc
CFLAGS = -Wall -Werror -Ilib
AR = ar
ARFLAGS = rcs

CUR_DIR = .
BIN_DIR = $(CUR_DIR)/bin
INC_DIR = $(CUR_DIR)/inc
LIB_DIR = $(CUR_DIR)/lib
OBJ_DIR = $(CUR_DIR)/obj
SRC_DIR = $(CUR_DIR)/src
LIBSTATIC_DIR = $(LIB_DIR)/libstatic
LIBSHARED_DIR = $(LIB_DIR)/libshared

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))


STATICLIB = $(LIBSTATIC_DIR)/libmain.a
SHAREDLIB = $(LIBSHARED_DIR)/libmain.so

MAIN = main.c
TARGET_STATIC = $(BIN_DIR)/main_static
TARGET_SHARED = $(BIN_DIR)/main_shared


all: $(TARGET_STATIC) $(TARGET_SHARED)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

#static lib
$(STATICLIB): $(OBJS)
	@mkdir -p $(LIBSTATIC_DIR)
	$(AR) $(ARFLAGS) $@ $^

#shared lib

$(SHAREDLIB): $(OBJS)
	@mkdir -p $(LIBSHARED_DIR)
	$(CC) -shared -o $@ $^

$(TARGET_STATIC): $(STATICLIB) $(MAIN)
	@mkdir -p $(BIN_DIR)
	$(CC) -Iinc $(MAIN) -L$(LIBSTATIC_DIR) -lmain -o $@

$(TARGET_SHARED): $(SHAREDLIB) $(MAIN)
	@mkdir -p $(BIN_DIR)
	$(CC) -Iinc $(MAIN) -L$(LIBSHARED_DIR) -lmain -o $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) $(SHAREDLIB_DIR) $(STATICLIB_DIR)

.PHONY: all clean













