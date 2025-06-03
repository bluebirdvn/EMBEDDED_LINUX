.PHONY: clean all

CC = gcc
CFLAGS = -Wall -Werror

SRC = main.c
TARGET = main
FILE = hello.txt
all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -rf $(TARGET) $(FILE) 
