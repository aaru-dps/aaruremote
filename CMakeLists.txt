cmake_minimum_required(VERSION 3.0)
project(dicremote C)
include(CheckIncludeFiles)
include(CheckLibraryExists)

set(CMAKE_C_STANDARD 90)

set(MAIN_SOURCES main.c list_devices.c device.c scsi.c hex2bin.c usb.c ieee1394.c pcmcia.c ata.c sdhci.c worker.c)

add_library(dicremote ${MAIN_SOURCES})

if (WII)
    set(CMAKE_C_FLAGS "-mrvl -mcpu=750 -meabi -mhard-float")
    add_definitions(-DGEKKO=1)
    include_directories($ENV{DEVKITPRO}/libogc/include/)
endif ()

set(DIC_PORTS "linux;wii" CACHE STRING "List of ports to build")
foreach (PORT IN LISTS DIC_PORTS)
    add_subdirectory(${PORT})
endforeach (PORT)