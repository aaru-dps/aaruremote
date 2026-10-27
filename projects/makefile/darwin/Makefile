# Makefile for building the Aaru Remote Server on macOS / Mac OS X without CMake.
# Works with the GNU make 3.81 shipped with Xcode.
#
# Usage:
#   make                    Build build/aaruremote
#   make BASELINE_ISA=no    Do not pin -march/-mcpu to the architecture baseline
#   make install            Install to $(DESTDIR)$(PREFIX)/bin
#   make clean              Remove the build directory
#
# CC, CFLAGS, CPPFLAGS, LDFLAGS and LDLIBS can be overridden as usual.

SRCDIR   := ../../..
BUILDDIR := build
TARGET   := $(BUILDDIR)/aaruremote

PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin

CC     ?= cc
CFLAGS ?= -O2 -g

BASELINE_ISA ?= yes

CORE_SOURCES  := hex2bin.c list_devices.c main.c worker.c
MACOS_SOURCES := ata.c device.c ieee1394.c list_devices.c pcmcia.c scsi.c sdhci.c usb.c
UNIX_SOURCES  := hello.c network.c unix.c

# Objects are kept per source directory, as list_devices.c exists in more than one
OBJECTS := $(addprefix $(BUILDDIR)/core/,$(CORE_SOURCES:.c=.o)) \
           $(addprefix $(BUILDDIR)/macos/,$(MACOS_SOURCES:.c=.o)) \
           $(addprefix $(BUILDDIR)/unix/,$(UNIX_SOURCES:.c=.o))

HEADERS := $(wildcard $(SRCDIR)/*.h $(SRCDIR)/macos/*.h $(SRCDIR)/unix/*.h)

# Version, matching the CMake build: <base>-<short git commit>, or <base>-unknown when git or its metadata is
# unavailable. git -C and command -v are avoided as old versions of git and sh do not support them.
VERSION_BASE := 1.0
GIT_COMMIT   := $(shell (cd $(SRCDIR) && git rev-parse --short HEAD) 2>/dev/null)
ifeq ($(GIT_COMMIT),)
VERSION := $(VERSION_BASE)-unknown
else
VERSION := $(VERSION_BASE)-$(GIT_COMMIT)
endif

# Old compilers do not know -std
AARU_CFLAGS   := $(shell echo 'int main(void){return 0;}' | \
                   $(CC) -std=gnu89 -x c - -o /dev/null >/dev/null 2>&1 && echo -std=gnu89)
AARU_CPPFLAGS := -DAARUREMOTE_VERSION=\"$(VERSION)\"
AARU_LIBS     := -framework IOKit -framework CoreFoundation

# There is no <endian.h>, so tell endian.h the byte order. Apple compilers define __BIG_ENDIAN__ on PowerPC.
BIG_ENDIAN := $(shell echo __BIG_ENDIAN__ | $(CC) -E -x c - 2>/dev/null | tail -n 1)
ifeq ($(BIG_ENDIAN),1)
AARU_CPPFLAGS += -D__BYTE_ORDER=4321
else
AARU_CPPFLAGS += -D__BYTE_ORDER=1234
endif

# Compile for the baseline instruction set of the target architecture, so binaries run on any CPU of it
ifeq ($(BASELINE_ISA),yes)
MACHINE := $(shell $(CC) -dumpmachine 2>/dev/null | cut -d- -f1)

ifneq ($(filter x86_64 amd64,$(MACHINE)),)
ISA_FLAG := -march=x86-64
else ifneq ($(filter i386 i486 i586 i686,$(MACHINE)),)
ISA_FLAG := -march=i386
else ifneq ($(filter aarch64 arm64,$(MACHINE)),)
ISA_FLAG := -march=armv8-a
else ifneq ($(filter ppc64 powerpc64,$(MACHINE)),)
ISA_FLAG := -mcpu=powerpc64
else ifneq ($(filter ppc powerpc,$(MACHINE)),)
ISA_FLAG := -mcpu=powerpc
endif

ifneq ($(ISA_FLAG),)
ISA_SUPPORTED := $(shell echo 'int main(void){return 0;}' | \
                   $(CC) $(ISA_FLAG) -x c - -o /dev/null >/dev/null 2>&1 && echo yes)

ifeq ($(ISA_SUPPORTED),yes)
AARU_CFLAGS += $(ISA_FLAG)
endif
endif
endif

.PHONY: all clean install

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(AARU_CFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJECTS) $(AARU_LIBS) $(LDLIBS)

$(BUILDDIR)/core/%.o: $(SRCDIR)/%.c $(HEADERS) | $(BUILDDIR)/core
	$(CC) $(AARU_CFLAGS) $(CFLAGS) $(AARU_CPPFLAGS) $(CPPFLAGS) -c -o $@ $<

$(BUILDDIR)/macos/%.o: $(SRCDIR)/macos/%.c $(HEADERS) | $(BUILDDIR)/macos
	$(CC) $(AARU_CFLAGS) $(CFLAGS) $(AARU_CPPFLAGS) $(CPPFLAGS) -c -o $@ $<

$(BUILDDIR)/unix/%.o: $(SRCDIR)/unix/%.c $(HEADERS) | $(BUILDDIR)/unix
	$(CC) $(AARU_CFLAGS) $(CFLAGS) $(AARU_CPPFLAGS) $(CPPFLAGS) -c -o $@ $<

$(BUILDDIR)/core $(BUILDDIR)/macos $(BUILDDIR)/unix:
	mkdir -p $@

install: $(TARGET)
	mkdir -p $(DESTDIR)$(BINDIR)
	cp $(TARGET) $(DESTDIR)$(BINDIR)/aaruremote
	chmod 755 $(DESTDIR)$(BINDIR)/aaruremote

clean:
	rm -rf $(BUILDDIR)
