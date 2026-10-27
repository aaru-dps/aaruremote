# Makefile for building the Aaru Remote Server on Linux without CMake.
#
# Usage:
#   make                    Build build/aaruremote
#   make UDEV=no            Build without libudev even if it is available
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

UDEV         ?= auto
BASELINE_ISA ?= yes

CORE_SOURCES  := hex2bin.c list_devices.c main.c worker.c
LINUX_SOURCES := ata.c device.c ieee1394.c list_devices.c pcmcia.c scsi.c sdhci.c usb.c
UNIX_SOURCES  := hello.c network.c unix.c

# Objects are kept per source directory, as list_devices.c exists in more than one
OBJECTS := $(addprefix $(BUILDDIR)/core/,$(CORE_SOURCES:.c=.o)) \
           $(addprefix $(BUILDDIR)/linux/,$(LINUX_SOURCES:.c=.o)) \
           $(addprefix $(BUILDDIR)/unix/,$(UNIX_SOURCES:.c=.o))

HEADERS := $(wildcard $(SRCDIR)/*.h $(SRCDIR)/linux/*.h $(SRCDIR)/linux/mmc/*.h $(SRCDIR)/unix/*.h)

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
AARU_CPPFLAGS := -DAARUREMOTE_VERSION=\"$(VERSION)\" -DHAVE_ENDIAN_H
AARU_LIBS     :=

# libudev, used to enumerate devices
ifeq ($(UDEV),auto)
HAVE_UDEV := $(shell echo 'int main(void){return udev_new()==0;}' | \
               $(CC) -include libudev.h -x c - -ludev -o /dev/null >/dev/null 2>&1 && echo yes)
else
HAVE_UDEV := $(filter yes,$(UDEV))
endif

ifeq ($(HAVE_UDEV),yes)
AARU_CPPFLAGS += -DHAS_UDEV
AARU_LIBS     += -ludev
endif

# SD/MMC commands need the kernel's MMC ioctl header
HAVE_MMC_IOCTL_H := $(shell echo | $(CC) -E -include linux/mmc/ioctl.h -x c - >/dev/null 2>&1 && echo yes)

ifeq ($(HAVE_MMC_IOCTL_H),yes)
AARU_CPPFLAGS += -DHAS_UAPI_MMC
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
else ifneq ($(filter arm%,$(MACHINE)),)
ISA_FLAG := -march=armv6
else ifneq ($(filter mips64%,$(MACHINE)),)
ISA_FLAG := -march=mips3
else ifneq ($(filter mips%,$(MACHINE)),)
ISA_FLAG := -march=mips2
else ifneq ($(filter ppc64% powerpc64%,$(MACHINE)),)
ISA_FLAG := -mcpu=powerpc64
else ifneq ($(filter ppc% powerpc%,$(MACHINE)),)
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

$(BUILDDIR)/linux/%.o: $(SRCDIR)/linux/%.c $(HEADERS) | $(BUILDDIR)/linux
	$(CC) $(AARU_CFLAGS) $(CFLAGS) $(AARU_CPPFLAGS) $(CPPFLAGS) -c -o $@ $<

$(BUILDDIR)/unix/%.o: $(SRCDIR)/unix/%.c $(HEADERS) | $(BUILDDIR)/unix
	$(CC) $(AARU_CFLAGS) $(CFLAGS) $(AARU_CPPFLAGS) $(CPPFLAGS) -c -o $@ $<

$(BUILDDIR)/core $(BUILDDIR)/linux $(BUILDDIR)/unix:
	mkdir -p $@

install: $(TARGET)
	mkdir -p $(DESTDIR)$(BINDIR)
	cp $(TARGET) $(DESTDIR)$(BINDIR)/aaruremote
	chmod 755 $(DESTDIR)$(BINDIR)/aaruremote

clean:
	rm -rf $(BUILDDIR)
