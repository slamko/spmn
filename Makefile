CC=gcc -g
SRC=$(wildcard *.c)
BIN=sise 
LIBS=-pthread
CFLAGS=-Wall -Wextra -Werror -pedantic

all: $(SRC)
	$(CC) $(CFLAGS) $(LIBS) $(SRC) -o $(BIN) 