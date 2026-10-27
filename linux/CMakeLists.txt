project(aaruremote-linux C)

if (NOT "${CMAKE_SYSTEM}" MATCHES "Linux")
    return()
endif ()

set(PLATFORM_SOURCES list_devices.c linux.h device.c scsi.c usb.c ieee1394.c pcmcia.c ata.c sdhci.c ../unix/hello.c
        ../unix/network.c ../unix/unix.c mmc/ioctl.h ../unix/unix.h)
CHECK_LIBRARY_EXISTS("udev" udev_new "" HAS_UDEV)
# Kernel headers before 4.4 have the MMC ioctls but not MMC_IOC_MULTI_CMD, use the bundled header with those
include(CheckSymbolExists)
CHECK_SYMBOL_EXISTS(MMC_IOC_MULTI_CMD "sys/ioctl.h;linux/mmc/ioctl.h" HAVE_MMC_IOC_MULTI_CMD)

add_executable(aaruremote ${PLATFORM_SOURCES})

if (HAS_UDEV)
    target_link_libraries(aaruremote udev)
    add_definitions(-DHAS_UDEV)
endif ()

if (HAVE_MMC_IOC_MULTI_CMD)
    add_definitions(-DHAS_UAPI_MMC)
endif ()

target_link_libraries(aaruremote aaruremotecore)