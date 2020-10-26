project(aaruremote-fbsd C)

if (NOT "${CMAKE_SYSTEM}" MATCHES "FreeBSD")
    return()
endif ()

set(PLATFORM_SOURCES list_devices.c freebsd.h device.c scsi.c usb.c ieee1394.c pcmcia.c ata.c sdhci.c ../unix/hello.c
        ../unix/network.c ../unix/unix.c ../unix/unix.h)

add_executable(aaruremote ${PLATFORM_SOURCES})

target_link_libraries(aaruremote aaruremotecore)