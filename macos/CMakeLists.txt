project(aaruremote-macos C)

if (NOT "${CMAKE_SYSTEM}" MATCHES "Darwin")
    return()
endif ()

find_library(IOKIT_FRAMEWORK IOKit)
find_library(COREFOUNDATION_FRAMEWORK CoreFoundation)

if (NOT IOKIT_FRAMEWORK)
    message(FATAL_ERROR "Cannot find IOKit framework.")
endif ()

if (NOT COREFOUNDATION_FRAMEWORK)
    message(FATAL_ERROR "Cannot find CoreFoundation framework.")
endif ()

set(PLATFORM_SOURCES list_devices.c macos.h device.c scsi.c usb.c ieee1394.c pcmcia.c ata.c sdhci.c
        ../unix/hello.c ../unix/network.c ../unix/unix.c ../unix/unix.h)

add_executable(aaruremote ${PLATFORM_SOURCES})

target_link_libraries(aaruremote aaruremotecore ${IOKIT_FRAMEWORK} ${COREFOUNDATION_FRAMEWORK})
