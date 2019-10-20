project(dicremote-wii C)

if (NOT WII)
    return()
endif ()

set(PLATFORM_SOURCES wii.c hello.c network.c wii.h)

add_executable(dicremote-wii ${PLATFORM_SOURCES})
set_target_properties(dicremote-wii PROPERTIES LINK_FLAGS -L$ENV{DEVKITPRO}/libogc/lib/wii/)
set_target_properties(dicremote-wii PROPERTIES OUTPUT_NAME dicremote.elf)
target_link_libraries(dicremote-wii dicremote -logc)