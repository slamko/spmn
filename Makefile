CC=gcc
TARGET=spmn
LIBS=-pthread -lm -lbsd 
WEFLAGS=-Wall -Wextra -Wno-unused-parameter -pedantic -Iinclude/ -I. -Izic/lib
CFLAGS=$(WEFLAGS) -g
VERSION=1.0_3
OPT=-O1
PKG_NAME=$(TARGET)-$(VERSION)

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

installdirs: release COPYING README.md
	mkdir -p $(PKG_NAME)/usr/share/licenses/$(TARGET)
	mkdir -p $(PKG_NAME)/usr/share/doc/$(TARGET)
	mkdir -p $(PKG_NAME)/usr/bin
	mkdir -p $(PKG_NAME)/usr/share/man/man1/
	cp COPYING $(PKG_NAME)/usr/share/licenses/$(TARGET)
	cp README.md $(PKG_NAME)/usr/share/doc/spm
	cp $(TARGET) $(PKG_NAME)/usr/bin
	cp $(TARGET).1 $(PKG_NAME)/usr/share/man/man1/

deb-installdirs: installdirs
	mkdir -p $(PKG_NAME)/DEBIAN
	cp ./packaging/deb/DEBIAN_control $(PKG_NAME)/DEBIAN/control

include ./packaging/deb/Makefile

include ./packaging/xbps/Makefile

.ONESHELL: test-aur-pkg

test-aur-pkg: PKGBUILD
	mkdir test-aur
	cp PKGBUILD ./test-aur
	cd test-aur
	makepkg
	cd -
	rm -rf test-aur

clean-pkgdirs:
	rm -rf $(PKG_NAME)

dist: dist-zip dist-gz

dist-zip:
	git archive HEAD --format=zip > $(PKG_NAME).zip

dist-gz:
	git archive HEAD --format=tar > $(PKG_NAME).tar
	gzip $(PKG_NAME).tar

gen-sha256sums:
	sha256sum $(PKG_NAME).tar.gz \
	 $(PKG_NAME).zip \
	 $(PKG_NAME).deb \
	 $(PKG_NAME).amd64.xbps \
	 $(PKG_NAME).x86_64.xbps > SHA256SUMS

install: release
	mkdir -p $(BIND)
	mkdir -p /usr/share/licenses/$(TARGET)
	mkdir -p /usr/share/doc/$(TARGET)
	mkdir -p /usr/share/man/man1/
	install -Dm755 ./$(TARGET) $(BIN)
	install -Dm644 ./COPYING /usr/share/licenses/$(TARGET)/COPYING
	install -Dm644 ./README.md /usr/share/doc/$(TARGET)/README
	install -Dm644 ./$(TARGET).1 /usr/share/man/man1/$(TARGET).1

uninstall:
	$(RM) $(BIN)
	$(RM) /usr/share/licenses/$(TARGET)/COPYING
	$(RM) /usr/share/doc/$(TARGET)/README
	$(RM) /usr/share/man/man1/$(TARGET).1

.PHONY: clean

clean:
	$(RM) -r $(BUILDD)	
	$(RM) $(TARGET)
	$(RM) *gz
	$(RM) *zip
