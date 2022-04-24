CC=gcc -g
TARGET=sise 
LIBS=-pthread -lm
CFLAGS=-Wall -Wextra -Werror -pedantic

SRCD=src
BUILDD=build
SRC=$(wildcard $(SRCD)/*.c)
OBJS=$(SRC:$(SRCD)/%.c=$(BUILDD)/%.o)

all: $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

$(BUILDD)/%.o: $(SRCD)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean: 
	$(RM) -r build/*
	$(RM) $(TARGET)