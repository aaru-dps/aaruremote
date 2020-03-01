project("aaruremote-win32" C)

if (NOT "${CMAKE_SYSTEM}" MATCHES "Windows")
    return()
endif ()

CHECK_INCLUDE_FILES("ntddscsi.h" HAVE_NTDDSCSI_H)
CHECK_INCLUDE_FILES("sffdisk.h" HAVE_SFFDISK_H)
CHECK_INCLUDE_FILES("sddef.h" HAVE_SDDEF_H)

CHECK_TYPE_SIZE(ATA_PASS_THROUGH_EX APTE)
CHECK_TYPE_SIZE(SCSI_PASS_THROUGH_DIRECT SPTD)
CHECK_TYPE_SIZE(SFFDISK_DEVICE_COMMAND_DATA SDCD)
CHECK_TYPE_SIZE(SDCMD_DESCRIPTOR SDCMDD)

if (HAVE_NTDDSCSI_H)
    add_definitions(-DHAS_NTDDSCSI_H)
endif ()

if (HAVE_SFFDISK_H)
    add_definitions(-DHAS_SFFDISK_H)
endif ()

if (HAVE_SDDEF_H)
    add_definitions(-DHAS_SDDEF_H)
endif ()

if (HAVE_APTE)
    add_definitions(-DHAS_APTE)
endif ()

if (HAVE_SPTD)
    add_definitions(-DHAS_SPTD)
endif ()

if (HAVE_SDCD)
    add_definitions(-DHAS_SDCD)
endif ()

if (HAVE_SDCMDD)
    add_definitions(-DHAS_SDCMDD)
endif ()

set(PLATFORM_SOURCES "win32.h" network.c hello.c "win32.c" list_devices.c ata.c device.c ieee1394.c pcmcia.c scsi.c sdhci.c usb.h usb.c ntioctl.h)

add_executable(aaruremote ${PLATFORM_SOURCES})

target_link_libraries(aaruremote aaruremotecore ws2_32 iphlpapi version setupapi cfgmgr32)