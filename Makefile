CC=gcc -g
SRC=$(wildcard src/*.c)
OBJS=$(SRC:src/%.c=build/%.o)
TARGET=sise 
LIBS=-pthread -lm
CFLAGS=-Wall -Wextra -Werror -pedantic
 
all: $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

build/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean: 
	rm -f build/*
	rm $(TARGET)