project("dicremote-win32" C)

if (NOT "${CMAKE_SYSTEM}" MATCHES "Windows")
    return()
endif ()

set(PLATFORM_SOURCES "win32.h" network.c hello.c "win32.c" list_devices.c ata.c device.c ieee1394.c pcmcia.c scsi.c sdhci.c usb.c)

add_executable(dicremote ${PLATFORM_SOURCES})

target_link_libraries(dicremote dicremotecore ws2_32 iphlpapi version)