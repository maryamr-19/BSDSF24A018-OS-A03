
CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude
LDFLAGS = -lreadline

SRC = src/main.c src/shell.c src/execute.c
OBJ = $(SRC:.c=.o)

TARGET = bin/myshell

all: $(TARGET)

$(TARGET): $(OBJ)
	mkdir -p bin
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LDFLAGS)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(TARGET)

.PHONY: all clean

