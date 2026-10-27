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

CORE_SOURCES=	hex2bin.c list_devices.c main.c worker.c
FREEBSD_SOURCES=ata.c device.c ieee1394.c list_devices.c pcmcia.c scsi.c sdhci.c usb.c
UNIX_SOURCES=	hello.c network.c unix.c

HEADERS=	${SRCDIR}/aaruremote.h ${SRCDIR}/endian.h ${SRCDIR}/freebsd/freebsd.h ${SRCDIR}/unix/unix.h

# Version, matching the CMake build: <base>-<short git commit>, or <base>-unknown when git or its metadata is
# unavailable. git -C is avoided as older git versions do not support it.
VERSION_BASE=	1.0
GIT_COMMIT!=	(command -v git >/dev/null 2>&1 && cd ${SRCDIR} && git rev-parse --short HEAD 2>/dev/null) || true

.if empty(GIT_COMMIT)
VERSION=	${VERSION_BASE}-unknown
.else
VERSION=	${VERSION_BASE}-${GIT_COMMIT}
.endif

AARU_CFLAGS=	-std=gnu89
AARU_CPPFLAGS=	-DAARUREMOTE_VERSION=\"${VERSION}\"
AARU_LIBS=	-lcam

# FreeBSD only has <endian.h> since 14.0, before that the byte order is taken from <sys/endian.h>
.if exists(/usr/include/endian.h)
AARU_CPPFLAGS+=	-DHAVE_ENDIAN_H
.else
BYTE_ORDER_VALUE!=	echo _BYTE_ORDER | ${CC} -E -include sys/endian.h -x c - 2>/dev/null | tail -n 1
.if ${BYTE_ORDER_VALUE} == "4321"
AARU_CPPFLAGS+=	-D__BYTE_ORDER=4321
.else
AARU_CPPFLAGS+=	-D__BYTE_ORDER=1234
.endif
.endif

# Compile for the baseline instruction set of the target architecture, so binaries run on any CPU of it
.if ${BASELINE_ISA} == "yes"
.if ${MACHINE_ARCH} == "amd64"
ISA_FLAG=	-march=x86-64
.elif ${MACHINE_ARCH} == "i386"
ISA_FLAG=	-march=i386
.elif ${MACHINE_ARCH} == "aarch64"
ISA_FLAG=	-march=armv8-a
.elif ${MACHINE_ARCH:Marm*} != ""
ISA_FLAG=	-march=armv6
.elif ${MACHINE_ARCH:Mmips64*} != ""
ISA_FLAG=	-march=mips3
.elif ${MACHINE_ARCH:Mmips*} != ""
ISA_FLAG=	-march=mips2
.elif ${MACHINE_ARCH:Mpowerpc64*} != ""
ISA_FLAG=	-mcpu=powerpc64
.elif ${MACHINE_ARCH:Mpowerpc*} != ""
ISA_FLAG=	-mcpu=powerpc
.endif

.if defined(ISA_FLAG)
ISA_SUPPORTED!=	(echo 'int main(void){return 0;}' | \
		    ${CC} ${ISA_FLAG} -x c - -o /dev/null >/dev/null 2>&1 && echo yes) || true
.if ${ISA_SUPPORTED} == "yes"
AARU_CFLAGS+=	${ISA_FLAG}
.endif
.endif
.endif

# Objects are kept per source directory, as list_devices.c exists in more than one
OBJECTS=
.for src in ${CORE_SOURCES}
OBJECTS+=	${BUILDDIR}/core/${src:R}.o
.endfor
.for src in ${FREEBSD_SOURCES}
OBJECTS+=	${BUILDDIR}/freebsd/${src:R}.o
.endfor
.for src in ${UNIX_SOURCES}
OBJECTS+=	${BUILDDIR}/unix/${src:R}.o
.endfor

.PHONY: all clean install

all: ${TARGET}

${TARGET}: ${OBJECTS}
	${CC} ${AARU_CFLAGS} ${CFLAGS} ${LDFLAGS} -o ${.TARGET} ${OBJECTS} ${AARU_LIBS} ${LDADD}

.for src in ${CORE_SOURCES}
${BUILDDIR}/core/${src:R}.o: ${SRCDIR}/${src} ${HEADERS}
	@mkdir -p ${BUILDDIR}/core
	${CC} ${AARU_CFLAGS} ${CFLAGS} ${AARU_CPPFLAGS} ${CPPFLAGS} -c -o ${.TARGET} ${SRCDIR}/${src}
.endfor

.for src in ${FREEBSD_SOURCES}
${BUILDDIR}/freebsd/${src:R}.o: ${SRCDIR}/freebsd/${src} ${HEADERS}
	@mkdir -p ${BUILDDIR}/freebsd
	${CC} ${AARU_CFLAGS} ${CFLAGS} ${AARU_CPPFLAGS} ${CPPFLAGS} -c -o ${.TARGET} ${SRCDIR}/freebsd/${src}
.endfor

.for src in ${UNIX_SOURCES}
${BUILDDIR}/unix/${src:R}.o: ${SRCDIR}/unix/${src} ${HEADERS}
	@mkdir -p ${BUILDDIR}/unix
	${CC} ${AARU_CFLAGS} ${CFLAGS} ${AARU_CPPFLAGS} ${CPPFLAGS} -c -o ${.TARGET} ${SRCDIR}/unix/${src}
.endfor

install: ${TARGET}
	mkdir -p ${DESTDIR}${BINDIR}
	cp ${TARGET} ${DESTDIR}${BINDIR}/aaruremote
	chmod 755 ${DESTDIR}${BINDIR}/aaruremote

clean:
	rm -rf ${BUILDDIR}
