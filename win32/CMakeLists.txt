project("dicremote-win32" C)

if (NOT "${CMAKE_SYSTEM}" MATCHES "Windows")
    return()
endif ()

set(PLATFORM_SOURCES "win32.h" network.c hello.c "win32.c")

add_executable(dicremote ${PLATFORM_SOURCES})

target_link_libraries(dicremote dicremotecore ws2_32)