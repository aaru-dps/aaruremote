cmake_minimum_required(VERSION 3.14)
project(dicremote C)
include(CheckIncludeFiles)
include(CheckLibraryExists)

set(CMAKE_C_STANDARD 90)

set(MAIN_SOURCES main.c list_devices.c device.c scsi.c hex2bin.c usb.c ieee1394.c pcmcia.c ata.c sdhci.c)

if ("${CMAKE_SYSTEM}" MATCHES "Linux")
    set(PLATFORM_SOURCES linux/list_devices.c linux/linux.h linux/device.c linux/scsi.c linux/usb.c linux/ieee1394.c linux/pcmcia.c linux/ata.c linux/sdhci.c)
    CHECK_LIBRARY_EXISTS("udev" udev_new "" HAS_UDEV)
    CHECK_INCLUDE_FILES("linux/mmc/ioctl.h" HAVE_MMC_IOCTL_H)
endif ()

add_executable(dicremote-${CMAKE_SYSTEM_PROCESSOR} ${MAIN_SOURCES} ${PLATFORM_SOURCES})

if (HAS_UDEV)
    target_link_libraries(dicremote-${CMAKE_SYSTEM_PROCESSOR} udev)
    add_definitions(-DHAS_UDEV)
endif ()

if (HAVE_MMC_IOCTL_H)
    add_definitions(-DHAS_UAPI_MMC)
endif ()

if ("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "x86_64")
    add_executable(dicremote-x86 ${MAIN_SOURCES} ${PLATFORM_SOURCES})
    set_target_properties(dicremote-x86 PROPERTIES LINK_FLAGS -m32)
    target_compile_options(dicremote-x86 PUBLIC -m32)

    if (HAS_UDEV)
        target_link_libraries(dicremote-x86 udev)
        add_definitions(-DHAS_UDEV)
    endif ()
endif ()
