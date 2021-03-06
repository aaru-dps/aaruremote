cmake_minimum_required(VERSION 2.8)
project(aaruremote C)
include(CheckIncludeFiles)
include(CheckLibraryExists)
include(TestBigEndian)
set(CMAKE_C_STANDARD 90)

set(MAIN_SOURCES aaruremote.h endian.h hex2bin.c list_devices.c main.c worker.c)

add_library(aaruremotecore ${MAIN_SOURCES})

CHECK_INCLUDE_FILES("endian.h" HAVE_ENDIAN_H)

if (HAVE_ENDIAN_H)
    add_definitions(-DHAVE_ENDIAN_H)
endif ()

if (WII)
    set(CMAKE_C_FLAGS "-mrvl -mcpu=750 -meabi -mhard-float")
    add_definitions(-DGEKKO=1)
    include_directories($ENV{DEVKITPRO}/libogc/include/)
    add_definitions(-D__BYTE_ORDER=4321)
else ()
    if (NOT HAVE_ENDIAN_H AND NOT HAVE_SYS_ENDIAN_H)
        TEST_BIG_ENDIAN(IS_BIG_ENDIAN)
        if (IS_BIG_ENDIAN)
            add_definitions(-D__BYTE_ORDER=4321)
        else ()
            add_definitions(-D__BYTE_ORDER=1234)
        endif ()
    endif ()
endif ()


set(AARU_PORTS "linux;wii;win32;freebsd" CACHE STRING "List of ports to build")
foreach (PORT IN LISTS AARU_PORTS)
    add_subdirectory(${PORT})
endforeach (PORT)

