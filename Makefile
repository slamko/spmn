CC=gcc -g
SRC=$(wildcard *.c)
BIN=sise 
LIBS=-pthread -lm
CFLAGS=-Wall -Wextra -Werror -pedantic

all: $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(LIBS) -o $(BIN) 