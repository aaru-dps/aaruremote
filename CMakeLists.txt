cmake_minimum_required(VERSION 3.14)
project(dicremote C)

set(CMAKE_C_STANDARD 90)

set(PLATFORM_SOURCES device.c)

if("${CMAKE_SYSTEM}" MATCHES "Linux")
    list(APPEND PLATFORM_SOURCES linux/list_devices.c linux/linux.h)
endif()

add_executable(dicremote main.c list_devices.c ${PLATFORM_SOURCES})

# TODO: Properly check udev exists
if("${CMAKE_SYSTEM}" MATCHES "Linux")
    target_link_libraries(dicremote udev)
endif()