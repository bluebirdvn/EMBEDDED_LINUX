CC = gcc
CFLAGS = -Wall -Werror -Wextra -g

TARGET = main
SRC = main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -rf $(TARGET)

.PHONY: clean all