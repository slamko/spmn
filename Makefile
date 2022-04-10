CC=gcc -g
SRC=$(wildcard *.c)
BIN=sise 
LIBS=-pthread
CFLAGS=-Wall -Werror -pedantic

all: $(SRC)
	$(CC) $(CFLAGS) $(LIBS) $(SRC) -o $(BIN) 