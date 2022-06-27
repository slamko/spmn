CC=gcc
TARGET=spm
LIBS=-pthread -lm -lbsd 
WEFLAGS=-Wall -Wextra -Wno-unused-parameter -Werror -pedantic
CFLAGS=$(WEFLAGS) -g -Iinclude/ -I.
VERSION=1.0_3

SRCD=src
HEADERD=include
BUILDD=build
UTILSD=utils
CMDD=commands

SRCMAIN:=$(wildcard $(SRCD)/*.c)
SRCUTILS:=$(wildcard $(SRCD)/$(UTILSD)/*.c)
SRCCOMMANDS:=$(wildcard $(SRCD)/$(CMDD)/*.c)
SRC=$(SRCMAIN) $(SRCUTILS) $(SRCCOMMANDS)
BIND=/bin
BIN:=$(BIND)/$(TARGET)

OBJS=$(SRC:$(SRCD)/%.c=$(BUILDD)/%.o)
OBJDIRS=$(BUILDD) $(BUILDD)/$(UTILSD) $(BUILDD)/$(CMDD)

HEADERSMAIN:=$(wildcard $(HEADERD)/*.h)
HEADERSUTILS:=$(SRCUTILS:$(SRCD)/%.c=$(HEADERD)/%.h)
HEADERSCMD:=$(SRCCOMMANDS:$(SRCD)/%.c=$(HEADERD)/%.h)
HEADERS=$(HEADERSMAIN) $(HEADERSUTILS) $(HEADERSCMD)

all: $(TARGET) $(OBJDIRS)

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

$(OBJDIRS):
	mkdir -p $@

$(BUILDD)/%.o:$(SRCD)/%.c $(HEADERSMAIN)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDD)/$(UTILSD)/%.o: $(SRCD)/$(UTILSD)/%.c $(HEADERSUTILS)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDD)/$(CMDD)/%.o: $(SRCD)/$(CMDD)/%.c $(HEADERSCMD)
	$(CC) $(CFLAGS) -c $< -o $@

install: $(TARGET) $(OBJDIRS)
	mkdir -p $(BIND)
	cp -f ./$(TARGET) $(BIN)
	chmod 755 $(BIN)

installdirs: $(TARGET) COPYING README.md
	mkdir -p $(TARGET)-$(VERSION)/usr/share/licenses/spm
	mkdir -p $(TARGET)-$(VERSION)/usr/share/doc/spm
	mkdir -p $(TARGET)-$(VERSION)/usr/bin
	mkdir -p $(TARGET)-$(VERSION)/usr/share/man/man1/
	cp COPYING $(TARGET)-$(VERSION)/usr/share/licenses/spm
	cp README.md $(TARGET)-$(VERSION)/usr/share/doc/spm
	cp $(TARGET) $(TARGET)-$(VERSION)/usr/bin
	cp spm.1 $(TARGET)-$(VERSION)/usr/share/man/man1/

xbps-clean:
	$(RM) x86_64-repodata
	$(RM) $(TARGET)-$(VERSION).xbps
	sudo xbps-remove spm

xbps-build: xbps-clean
	./xbps-create.sh
	xbps-rindex -a *.xbps
	sudo xbps-install --repository=$(shell pwd) spm

uninstall:
	$(RM) $(BIN)

.PHONY: clean

clean: 
	$(RM) $(addsuffix /*.o, $(OBJDIRS))
	$(RM) $(TARGET)
