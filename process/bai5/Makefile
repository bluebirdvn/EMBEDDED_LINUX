CC = gcc
CFLAGS = -Wall -Werror

SRC = main.c
TARGET = main

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -rf $(TARGET) hello.txt

.PHONY: clean all

