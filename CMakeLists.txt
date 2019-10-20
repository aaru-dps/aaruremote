cmake_minimum_required(VERSION 3.14)
project(dicremote C)

set(CMAKE_C_STANDARD 90)

set(MAIN_SOURCES main.c list_devices.c device.c scsi.c hex2bin.c usb.c ieee1394.c pcmcia.c ata.c sdhci.c)

if ("${CMAKE_SYSTEM}" MATCHES "Linux")
    set(PLATFORM_SOURCES linux/list_devices.c linux/linux.h linux/device.c linux/scsi.c linux/usb.c linux/ieee1394.c linux/pcmcia.c linux/ata.c linux/sdhci.c)
    find_library(HAS_UDEV NAMES udev)
endif ()

add_executable(dicremote ${MAIN_SOURCES} ${PLATFORM_SOURCES})

if (HAS_UDEV)
    target_link_libraries(dicremote udev)
    add_definitions(-DHAS_UDEV)
endif ()