project("dicremote-win32" C)

if (NOT "${CMAKE_SYSTEM}" MATCHES "Windows")
    return()
endif ()

CHECK_INCLUDE_FILES("ntddscsi.h" HAVE_NTDDSCSI_H)
CHECK_TYPE_SIZE(ATA_PASS_THROUGH_EX APTE)
CHECK_TYPE_SIZE(SCSI_PASS_THROUGH_DIRECT SPTD)

if (HAVE_NTDDSCSI_H)
    add_definitions(-DHAS_NTDDSCSI_H)
endif ()

if (HAVE_APTE)
    add_definitions(-DHAS_APTE)
endif ()

if (HAVE_SPTD)
    add_definitions(-DHAS_SPTD)
endif ()

set(PLATFORM_SOURCES "win32.h" network.c hello.c "win32.c" list_devices.c ata.c device.c ieee1394.c pcmcia.c scsi.c sdhci.c usb.c ntioctl.h)

add_executable(dicremote ${PLATFORM_SOURCES})

target_link_libraries(dicremote dicremotecore ws2_32 iphlpapi version)