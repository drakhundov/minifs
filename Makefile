CC = clang
CFLAGS = -Wall -Werror -Iinclude

TARGET = build/bin/main

SRCS = $(wildcard src/*.c)

OBJS = $(patsubst src/%.c, build/obj/%.o, $(SRCS))
all: $(TARGET)

build/obj/%.o: src/%.c
	@mkdir -p build/obj
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	@mkdir -p build/bin
	$(CC) $(CFLAGS) -o $@ $^

run:
	./$(TARGET) file.txt

clean:
	@rm -rf build/*

.PHONY: all clean run