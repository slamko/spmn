CC=gcc
TARGET=spmn
LIBS=-pthread -lm -lbsd 
WEFLAGS=-Wall -Wextra -Wno-unused-parameter -Werror -pedantic -Iinclude/ -I. -Izic/lib
CFLAGS=$(WEFLAGS) -g
VERSION=1.0_3
OPT=-O1

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

all: release

executable: $(OBJDIRS) $(TARGET)

release: CFLAGS=$(WEFLAGS) $(OPT)
release: executable

debug: executable

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

installdirs: executable COPYING README.md
	mkdir -p $(TARGET)-$(VERSION)/usr/share/licenses/$(TARGET)
	mkdir -p $(TARGET)-$(VERSION)/usr/share/doc/$(TARGET)
	mkdir -p $(TARGET)-$(VERSION)/usr/bin
	mkdir -p $(TARGET)-$(VERSION)/usr/share/man/man1/
	cp COPYING $(TARGET)-$(VERSION)/usr/share/licenses/$(TARGET)
	cp README.md $(TARGET)-$(VERSION)/usr/share/doc/spm
	cp $(TARGET) $(TARGET)-$(VERSION)/usr/bin
	cp $(TARGET).1 $(TARGET)-$(VERSION)/usr/share/man/man1/

deb-installdirs: installdirs
	mkdir -p $(TARGET)-$(VERSION)/DEBIAN
	cp ./packaging/deb/DEBIAN_control $(TARGET)-$(VERSION)/DEBIAN/control

include ./packaging/deb/Makefile

include ./packaging/xbps/Makefile

clean-pkgdirs:
	rm -rf $(TARGET)-$(VERSION)

install: release
	mkdir -p $(BIND)
	mkdir -p /usr/share/licenses/$(TARGET)
	mkdir -p /usr/share/doc/$(TARGET)
	mkdir -p /usr/share/man/man1/
	cp -f ./$(TARGET) $(BIN)
	cp -f ./COPYING /usr/share/licenses/$(TARGET)/COPYING
	cp -f ./README.md /usr/share/doc/$(TARGET)/README
	cp -f ./$(TARGET).1 /usr/share/man/man1/$(TARGET).1
	chmod 755 $(BIN)

uninstall:
	$(RM) $(BIN)
	$(RM) /usr/share/licenses/$(TARGET)/COPYING
	$(RM) /usr/share/doc/$(TARGET)/README
	$(RM) /usr/share/man/man1/$(TARGET).1

.PHONY: clean

clean:
	$(RM) -r $(BUILDD)	
	$(RM) $(TARGET)
