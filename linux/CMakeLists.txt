project(dicremote-linux C)

if (NOT "${CMAKE_SYSTEM}" MATCHES "Linux")
    return()
endif ()

set(PLATFORM_SOURCES list_devices.c linux.h device.c scsi.c usb.c ieee1394.c pcmcia.c ata.c sdhci.c ../unix/hello.c
        ../unix/network.c ../unix/unix.c mmc/ioctl.h)
CHECK_LIBRARY_EXISTS("udev" udev_new "" HAS_UDEV)
CHECK_INCLUDE_FILES("linux/mmc/ioctl.h" HAVE_MMC_IOCTL_H)

add_executable(dicremote ${PLATFORM_SOURCES})

if (HAS_UDEV)
    target_link_libraries(dicremote udev)
    add_definitions(-DHAS_UDEV)
endif ()

if (HAVE_MMC_IOCTL_H)
    add_definitions(-DHAS_UAPI_MMC)
endif ()

target_link_libraries(dicremote dicremotecore)