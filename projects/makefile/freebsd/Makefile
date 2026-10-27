# BSD make Makefile for building the Aaru Remote Server on FreeBSD without CMake.
#
# Usage:
#   make                    Build build/aaruremote
#   make BASELINE_ISA=no    Do not pin -march/-mcpu to the architecture baseline
#   make install            Install to ${DESTDIR}${PREFIX}/bin
#   make clean              Remove the build directory
#
# CC, CFLAGS, CPPFLAGS, LDFLAGS and LDADD can be overridden as usual.

SRCDIR=		${.CURDIR}/../../..
BUILDDIR=	${.CURDIR}/build
TARGET=		${BUILDDIR}/aaruremote

PREFIX?=	/usr/local
BINDIR?=	${PREFIX}/bin

CC?=		cc
CFLAGS?=	-O2 -g

BASELINE_ISA?=	yes

HEADERS=	${SRCDIR}/aaruremote.h ${SRCDIR}/endian.h ${SRCDIR}/freebsd/freebsd.h ${SRCDIR}/unix/unix.h

# Version, matching the CMake build: <base>-<short git commit>, or <base>-unknown when git or its metadata is
# unavailable. git -C and command -v are avoided as old versions of git and sh do not support them.
VERSION_BASE=	1.0
GIT_COMMIT!=	(cd ${SRCDIR} && git rev-parse --short HEAD) 2>/dev/null || true

.if empty(GIT_COMMIT)
VERSION=	${VERSION_BASE}-unknown
.else
VERSION=	${VERSION_BASE}-${GIT_COMMIT}
.endif

# Old compilers, such as the gcc 2 of early FreeBSD releases, do not know -std
STD_SUPPORTED!=	(echo 'int main(void){return 0;}' | \
		    ${CC} -std=gnu89 -x c - -o /dev/null >/dev/null 2>&1 && echo yes) || true
.if "${STD_SUPPORTED}" == "yes"
AARU_CFLAGS=	-std=gnu89
.else
AARU_CFLAGS=
.endif
AARU_CPPFLAGS=	-DAARUREMOTE_VERSION=\"${VERSION}\"
AARU_LIBS=	-lcam

# FreeBSD only has <endian.h> since 14.0, before that the byte order is taken from <sys/endian.h>
.if exists(/usr/include/endian.h)
AARU_CPPFLAGS+=	-DHAVE_ENDIAN_H
.else
BYTE_ORDER_VALUE!=	echo _BYTE_ORDER | ${CC} -E -include sys/endian.h -x c - 2>/dev/null | tail -n 1
.if "${BYTE_ORDER_VALUE}" == "4321"
AARU_CPPFLAGS+=	-D__BYTE_ORDER=4321
.else
AARU_CPPFLAGS+=	-D__BYTE_ORDER=1234
.endif
.endif

# Compile for the baseline instruction set of the target architecture, so binaries run on any CPU of it
.if "${BASELINE_ISA}" == "yes"
.if "${MACHINE_ARCH}" == "amd64"
ISA_FLAG=	-march=x86-64
.elif "${MACHINE_ARCH}" == "i386"
ISA_FLAG=	-march=i386
.elif "${MACHINE_ARCH}" == "aarch64"
ISA_FLAG=	-march=armv8-a
.elif "${MACHINE_ARCH:Marm*}" != ""
ISA_FLAG=	-march=armv6
.elif "${MACHINE_ARCH:Mmips64*}" != ""
ISA_FLAG=	-march=mips3
.elif "${MACHINE_ARCH:Mmips*}" != ""
ISA_FLAG=	-march=mips2
.elif "${MACHINE_ARCH:Mpowerpc64*}" != ""
ISA_FLAG=	-mcpu=powerpc64
.elif "${MACHINE_ARCH:Mpowerpc*}" != ""
ISA_FLAG=	-mcpu=powerpc
.endif

.if defined(ISA_FLAG)
ISA_SUPPORTED!=	(echo 'int main(void){return 0;}' | \
		    ${CC} ${ISA_FLAG} -x c - -o /dev/null >/dev/null 2>&1 && echo yes) || true
.if "${ISA_SUPPORTED}" == "yes"
AARU_CFLAGS+=	${ISA_FLAG}
.endif
.endif
.endif

# Objects are kept per source directory, as list_devices.c exists in more than one. Rules are written out
# explicitly, as old versions of make expand all the variables in loops when parsing them.
OBJECTS=	\
	${BUILDDIR}/core/hex2bin.o \
	${BUILDDIR}/core/list_devices.o \
	${BUILDDIR}/core/main.o \
	${BUILDDIR}/core/worker.o \
	${BUILDDIR}/freebsd/ata.o \
	${BUILDDIR}/freebsd/device.o \
	${BUILDDIR}/freebsd/ieee1394.o \
	${BUILDDIR}/freebsd/list_devices.o \
	${BUILDDIR}/freebsd/pcmcia.o \
	${BUILDDIR}/freebsd/scsi.o \
	${BUILDDIR}/freebsd/sdhci.o \
	${BUILDDIR}/freebsd/usb.o \
	${BUILDDIR}/unix/hello.o \
	${BUILDDIR}/unix/network.o \
	${BUILDDIR}/unix/unix.o

.PHONY: all clean install

all: ${TARGET}

${TARGET}: ${OBJECTS}
	${CC} ${AARU_CFLAGS} ${CFLAGS} ${LDFLAGS} -o ${TARGET} ${OBJECTS} ${AARU_LIBS} ${LDADD}

${BUILDDIR}/core/hex2bin.o: ${SRCDIR}/hex2bin.c ${HEADERS}
	@mkdir -p ${BUILDDIR}/core
	${CC} ${AARU_CFLAGS} ${CFLAGS} ${AARU_CPPFLAGS} ${CPPFLAGS} -c -o ${BUILDDIR}/core/hex2bin.o ${SRCDIR}/hex2bin.c

${BUILDDIR}/core/list_devices.o: ${SRCDIR}/list_devices.c ${HEADERS}
	@mkdir -p ${BUILDDIR}/core
	${CC} ${AARU_CFLAGS} ${CFLAGS} ${AARU_CPPFLAGS} ${CPPFLAGS} -c -o ${BUILDDIR}/core/list_devices.o ${SRCDIR}/list_devices.c

${BUILDDIR}/core/main.o: ${SRCDIR}/main.c ${HEADERS}
	@mkdir -p ${BUILDDIR}/core
	${CC} ${AARU_CFLAGS} ${CFLAGS} ${AARU_CPPFLAGS} ${CPPFLAGS} -c -o ${BUILDDIR}/core/main.o ${SRCDIR}/main.c

${BUILDDIR}/core/worker.o: ${SRCDIR}/worker.c ${HEADERS}
	@mkdir -p ${BUILDDIR}/core
	${CC} ${AARU_CFLAGS} ${CFLAGS} ${AARU_CPPFLAGS} ${CPPFLAGS} -c -o ${BUILDDIR}/core/worker.o ${SRCDIR}/worker.c

${BUILDDIR}/freebsd/ata.o: ${SRCDIR}/freebsd/ata.c ${HEADERS}
	@mkdir -p ${BUILDDIR}/freebsd
	${CC} ${AARU_CFLAGS} ${CFLAGS} ${AARU_CPPFLAGS} ${CPPFLAGS} -c -o ${BUILDDIR}/freebsd/ata.o ${SRCDIR}/freebsd/ata.c

${BUILDDIR}/freebsd/device.o: ${SRCDIR}/freebsd/device.c ${HEADERS}
	@mkdir -p ${BUILDDIR}/freebsd
	${CC} ${AARU_CFLAGS} ${CFLAGS} ${AARU_CPPFLAGS} ${CPPFLAGS} -c -o ${BUILDDIR}/freebsd/device.o ${SRCDIR}/freebsd/device.c

${BUILDDIR}/freebsd/ieee1394.o: ${SRCDIR}/freebsd/ieee1394.c ${HEADERS}
	@mkdir -p ${BUILDDIR}/freebsd
	${CC} ${AARU_CFLAGS} ${CFLAGS} ${AARU_CPPFLAGS} ${CPPFLAGS} -c -o ${BUILDDIR}/freebsd/ieee1394.o ${SRCDIR}/freebsd/ieee1394.c

${BUILDDIR}/freebsd/list_devices.o: ${SRCDIR}/freebsd/list_devices.c ${HEADERS}
	@mkdir -p ${BUILDDIR}/freebsd
	${CC} ${AARU_CFLAGS} ${CFLAGS} ${AARU_CPPFLAGS} ${CPPFLAGS} -c -o ${BUILDDIR}/freebsd/list_devices.o ${SRCDIR}/freebsd/list_devices.c

${BUILDDIR}/freebsd/pcmcia.o: ${SRCDIR}/freebsd/pcmcia.c ${HEADERS}
	@mkdir -p ${BUILDDIR}/freebsd
	${CC} ${AARU_CFLAGS} ${CFLAGS} ${AARU_CPPFLAGS} ${CPPFLAGS} -c -o ${BUILDDIR}/freebsd/pcmcia.o ${SRCDIR}/freebsd/pcmcia.c

${BUILDDIR}/freebsd/scsi.o: ${SRCDIR}/freebsd/scsi.c ${HEADERS}
	@mkdir -p ${BUILDDIR}/freebsd
	${CC} ${AARU_CFLAGS} ${CFLAGS} ${AARU_CPPFLAGS} ${CPPFLAGS} -c -o ${BUILDDIR}/freebsd/scsi.o ${SRCDIR}/freebsd/scsi.c

${BUILDDIR}/freebsd/sdhci.o: ${SRCDIR}/freebsd/sdhci.c ${HEADERS}
	@mkdir -p ${BUILDDIR}/freebsd
	${CC} ${AARU_CFLAGS} ${CFLAGS} ${AARU_CPPFLAGS} ${CPPFLAGS} -c -o ${BUILDDIR}/freebsd/sdhci.o ${SRCDIR}/freebsd/sdhci.c

${BUILDDIR}/freebsd/usb.o: ${SRCDIR}/freebsd/usb.c ${HEADERS}
	@mkdir -p ${BUILDDIR}/freebsd
	${CC} ${AARU_CFLAGS} ${CFLAGS} ${AARU_CPPFLAGS} ${CPPFLAGS} -c -o ${BUILDDIR}/freebsd/usb.o ${SRCDIR}/freebsd/usb.c

${BUILDDIR}/unix/hello.o: ${SRCDIR}/unix/hello.c ${HEADERS}
	@mkdir -p ${BUILDDIR}/unix
	${CC} ${AARU_CFLAGS} ${CFLAGS} ${AARU_CPPFLAGS} ${CPPFLAGS} -c -o ${BUILDDIR}/unix/hello.o ${SRCDIR}/unix/hello.c

${BUILDDIR}/unix/network.o: ${SRCDIR}/unix/network.c ${HEADERS}
	@mkdir -p ${BUILDDIR}/unix
	${CC} ${AARU_CFLAGS} ${CFLAGS} ${AARU_CPPFLAGS} ${CPPFLAGS} -c -o ${BUILDDIR}/unix/network.o ${SRCDIR}/unix/network.c

${BUILDDIR}/unix/unix.o: ${SRCDIR}/unix/unix.c ${HEADERS}
	@mkdir -p ${BUILDDIR}/unix
	${CC} ${AARU_CFLAGS} ${CFLAGS} ${AARU_CPPFLAGS} ${CPPFLAGS} -c -o ${BUILDDIR}/unix/unix.o ${SRCDIR}/unix/unix.c

install: ${TARGET}
	mkdir -p ${DESTDIR}${BINDIR}
	cp ${TARGET} ${DESTDIR}${BINDIR}/aaruremote
	chmod 755 ${DESTDIR}${BINDIR}/aaruremote

clean:
	rm -rf ${BUILDDIR}
