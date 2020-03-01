project(aaruremote-wii C)

if (NOT WII)
    return()
endif ()

set(PLATFORM_SOURCES wii.c hello.c network.c wii.h list_devices.c unsupported.c unimplemented.c)

add_executable(aaruremote-wii ${PLATFORM_SOURCES})
set_target_properties(aaruremote-wii PROPERTIES LINK_FLAGS -L$ENV{DEVKITPRO}/libogc/lib/wii/)
set_target_properties(aaruremote-wii PROPERTIES OUTPUT_NAME aaruremote.elf)
target_link_libraries(aaruremote-wii aaruremotecore -lwiiuse -lbte -logc -lm)