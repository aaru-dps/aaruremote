cmake_minimum_required(VERSION 2.8)
project(aaruremote C)
include(CheckIncludeFiles)
include(CheckLibraryExists)
include(TestBigEndian)
set(CMAKE_C_STANDARD 90)

set(AARUREMOTE_VERSION_BASE     "1.0")
set(AARUREMOTE_VERSION_FALLBACK "${AARUREMOTE_VERSION_BASE}-unknown"
    CACHE STRING "Fallback aaruremote version when git metadata is unavailable")
set(AARUREMOTE_VERSION          "${AARUREMOTE_VERSION_FALLBACK}")

find_package(Git QUIET)

if(GIT_FOUND)
    execute_process(COMMAND "${GIT_EXECUTABLE}" rev-parse --short HEAD
                    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                    RESULT_VARIABLE AARUREMOTE_GIT_COMMIT_RESULT
                    OUTPUT_VARIABLE AARUREMOTE_GIT_COMMIT_OUTPUT
                    ERROR_QUIET
                    OUTPUT_STRIP_TRAILING_WHITESPACE)

    if(AARUREMOTE_GIT_COMMIT_RESULT EQUAL 0 AND NOT "${AARUREMOTE_GIT_COMMIT_OUTPUT}" STREQUAL "")
        set(AARUREMOTE_VERSION "${AARUREMOTE_VERSION_BASE}-${AARUREMOTE_GIT_COMMIT_OUTPUT}")
    endif ()
endif ()

set(AARUREMOTE_VERSION_C "${AARUREMOTE_VERSION}")
string(REPLACE "\\" "\\\\" AARUREMOTE_VERSION_C "${AARUREMOTE_VERSION_C}")
string(REPLACE "\"" "\\\"" AARUREMOTE_VERSION_C "${AARUREMOTE_VERSION_C}")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/aaruremote_version.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/aaruremote_version.h @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_definitions(-DHAVE_AARUREMOTE_VERSION_H)
message(STATUS "aaruremote version: ${AARUREMOTE_VERSION}")

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

    option(AARUREMOTE_BASELINE_ISA "Compile for the baseline instruction set of the target CPU architecture, ignoring any host-native tuning" ON)

    if (AARUREMOTE_BASELINE_ISA AND CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        include(CheckCCompilerFlag)

        string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" AARU_PROCESSOR_LOWER)

        if (AARU_PROCESSOR_LOWER MATCHES "^(x86_64|amd64)$")
            set(AARU_BASELINE_ISA_FLAG "-march=x86-64")
        elseif (AARU_PROCESSOR_LOWER MATCHES "^(i386|i486|i586|i686|x86)$")
            set(AARU_BASELINE_ISA_FLAG "-march=i386")
        elseif (AARU_PROCESSOR_LOWER MATCHES "^(aarch64|arm64)")
            set(AARU_BASELINE_ISA_FLAG "-march=armv8-a")
        elseif (AARU_PROCESSOR_LOWER MATCHES "^arm")
            set(AARU_BASELINE_ISA_FLAG "-march=armv6")
        elseif (AARU_PROCESSOR_LOWER MATCHES "^mips64")
            set(AARU_BASELINE_ISA_FLAG "-march=mips3")
        elseif (AARU_PROCESSOR_LOWER MATCHES "^mips")
            set(AARU_BASELINE_ISA_FLAG "-march=mips2")
        elseif (AARU_PROCESSOR_LOWER MATCHES "^(ppc64|powerpc64)")
            set(AARU_BASELINE_ISA_FLAG "-mcpu=powerpc64")
        elseif (AARU_PROCESSOR_LOWER MATCHES "^(ppc|powerpc)")
            set(AARU_BASELINE_ISA_FLAG "-mcpu=powerpc")
        endif ()

        if (AARU_BASELINE_ISA_FLAG)
            string(REGEX REPLACE "[^A-Za-z0-9]" "_" AARU_BASELINE_ISA_FLAG_VAR "HAVE_C_FLAG_${AARU_BASELINE_ISA_FLAG}")
            CHECK_C_COMPILER_FLAG("${AARU_BASELINE_ISA_FLAG}" ${AARU_BASELINE_ISA_FLAG_VAR})

            if (${AARU_BASELINE_ISA_FLAG_VAR})
                set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${AARU_BASELINE_ISA_FLAG}")
                message(STATUS "aaruremote: targeting baseline ISA with ${AARU_BASELINE_ISA_FLAG}")
            endif ()
        endif ()
    endif ()
endif ()


set(AARU_PORTS "linux;wii;win32;freebsd;macos" CACHE STRING "List of ports to build")
foreach (PORT IN LISTS AARU_PORTS)
    add_subdirectory(${PORT})
endforeach (PORT)

