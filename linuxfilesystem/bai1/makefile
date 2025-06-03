.PHONY: clean all
CC = gcc
CFLAGS = -Wall -Werror
FILE = hello.txt

SRC = main.c
TARGET = main

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -rf $(TARGET) $(FILE)

