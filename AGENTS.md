# AGENTS.md - AI Coding Agent Guidelines for Aaru Remote Server

## Project Overview

Aaru Remote Server (`aaruremote`) is a slim C application that acts as a network bridge between a remote [Aaru](https://github.com/aaru-dps/Aaru) instance and local storage devices. It receives device commands over TCP/IP (port 6666), forwards them to local devices, and returns the data to the client.

### Purpose
- Enable Aaru (a .NET application) to access devices on systems where .NET cannot run
- Support legacy systems and older Linux distributions
- Support embedded/specialized platforms (e.g., Nintendo Wii)

### License
GNU General Public License v3 (GPLv3)

## Architecture

### Core Components (Platform-Independent)
- `aaruremote.h` - Main header file with protocol definitions, packet structures, and function declarations
- `main.c` - Entry point, initialization, and startup
- `worker.c` - Main network worker loop handling client connections and protocol processing
- `list_devices.c` - Device enumeration (common parts)
- `hex2bin.c` - Hex string to binary conversion utility
- `endian.h` - Endianness handling macros

### Platform-Specific Directories
Each platform has its own directory with implementations of device access:

- **`linux/`** - Linux implementation
  - Device access via `/dev/sg*`, `/dev/sd*`
  - Uses `ioctl()` for SCSI, ATA, SD/MMC commands
  - Optional `libudev` support for device enumeration

- **`freebsd/`** - FreeBSD implementation
  - CAM (Common Access Method) for SCSI/ATA access

- **`win32/`** - Windows implementation
  - Uses `DeviceIoControl()` with `SCSI_PASS_THROUGH_DIRECT`, `ATA_PASS_THROUGH_EX`
  - Targets Windows NT 4.0 and higher (tested on Windows XP and higher)

- **`wii/`** - Nintendo Wii homebrew implementation
  - Uses libogc for network and device access

- **`unix/`** - Shared Unix utilities
  - Network setup, hello packet generation, common Unix functions

## Build System

### CMake-Based Build
```bash
# Standard build
cmake .
make

# Or use the build script
./build.sh
```

### Build Configuration
- Minimum CMake version: 2.8
- C Standard: C90 (ANSI C)
- Configured ports: `linux`, `wii`, `win32`, `freebsd`

### Cross-Compilation
- Nintendo Wii: Requires devkitPro toolchain, uses `wii.cmake` toolchain file
- Windows: Can be built with MSVC (projects in `projects/` directory) or MinGW

## Protocol

The Aaru Remote Protocol is a binary protocol defined in `aaruremote.h`:

### Key Constants
- `AARUREMOTE_PORT`: 6666
- `AARUREMOTE_REMOTE_ID`: 0x52434944 ("DICR")
- `AARUREMOTE_PACKET_ID`: 0x544B4350 ("PCKT")

### Packet Types
- Hello handshake (type 1)
- List devices (types 2-3)
- Open/Close device (types 4, 25)
- SCSI commands (types 5-6)
- ATA CHS/LBA28/LBA48 commands (types 7-12)
- SD/MMC commands (types 13-14, 28-29)
- Device info queries (USB, FireWire, PCMCIA, SD/MMC registers)
- OS-level read (types 31-32)

### Packet Structure
All packets start with `AaruPacketHeader`:
```c
typedef struct {
    uint32_t remote_id;    // AARUREMOTE_REMOTE_ID
    uint32_t packet_id;    // AARUREMOTE_PACKET_ID
    uint32_t len;          // Packet length
    uint8_t  version;      // Protocol version
    int8_t   packet_type;  // Packet type identifier
    char     spare[2];
} AaruPacketHeader;
```

## Device Types Supported
- ATA devices (CHS, LBA28, LBA48 addressing)
- ATAPI devices
- SCSI devices
- Secure Digital (SD) cards
- MultiMediaCard (MMC)
- NVMe (limited)

## Coding Conventions

### Style

Code style is configured in `.editorconfig` and `.clang-format`. Key conventions:

- C90 (ANSI C) compatible code
- 4-space indentation (tabs not used)
- Allman brace style (braces on next line, not indented)
- PascalCase for functions (e.g., `DeviceOpen`, `SendScsiCommand`)
- snake_case for variables and parameters (e.g., `device_ctx`, `error_registers`, `sense_buf`)
- ALL_CAPS for constants with `AARUREMOTE_` prefix
- Packed structures for network protocol (`#pragma pack`)
- Max line length: 120 characters
- Pointer alignment: right (e.g., `char *ptr`)
- No space before parentheses in function calls
- Align consecutive assignments and declarations across comments

### Platform Abstraction
- Core protocol logic in `worker.c`
- Platform-specific code isolated in respective directories
- Common function signatures defined in `aaruremote.h`
- Platform headers (e.g., `linux.h`, `win32.h`) for internal platform-specific declarations

### Memory Management
- Use standard `malloc()`/`free()`
- All network packet structures are packed
- Buffer sizes defined by protocol constants

## Key Functions to Implement (Per Platform)

When adding a new platform, implement these functions:

```c
// Device enumeration
DeviceInfoList* ListDevices();

// Device lifecycle
void* DeviceOpen(const char* device_path);
void  DeviceClose(void* device_ctx);
int32_t ReOpen(void* device_ctx, uint32_t* closeFailed);

// Device information
int32_t GetDeviceType(void* device_ctx);
uint8_t GetUsbData(...);
uint8_t GetFireWireData(...);
uint8_t GetPcmciaData(...);
int32_t GetSdhciRegisters(...);

// Command execution
int32_t SendScsiCommand(...);
int32_t SendAtaChsCommand(...);
int32_t SendAtaLba28Command(...);
int32_t SendAtaLba48Command(...);
int32_t SendSdhciCommand(...);
int32_t SendMultiSdhciCommand(...);
int32_t OsRead(...);

// System utilities
uint8_t AmIRoot();
AaruPacketHello* GetHello();
int PrintNetworkAddresses();
void Initialize();
void PlatformLoop(AaruPacketHello* pkt_server_hello);

// Network abstraction
void* NetSocket(...);
int32_t NetBind(...);
int32_t NetListen(...);
void* NetAccept(...);
int32_t NetRecv(...);
int32_t NetWrite(...);
int32_t NetClose(...);
```

## Testing

- Run as root for full device access
- Test with Aaru client using `remote` command
- Verify device listing with `list-devices` command
- URI format: `aaru://<IP ADDRESS>/<DEVICE PATH>`

## Dependencies

### Linux
- Optional: `libudev` for device enumeration
- Optional: `linux/mmc/ioctl.h` for SD/MMC support

### Windows
- `ws2_32.lib` (Winsock)
- `iphlpapi.lib` (IP Helper)
- `version.lib`
- `setupapi.lib`
- `cfgmgr32.lib`

### Nintendo Wii
- devkitPro/libogc

## File Structure Summary

```
aaruremote/
├── aaruremote.h       # Main header (protocol, structures, declarations)
├── endian.h           # Endianness macros
├── main.c             # Entry point
├── worker.c           # Network worker loop
├── list_devices.c     # Common device listing
├── hex2bin.c          # Utility functions
├── CMakeLists.txt     # Main CMake configuration
├── build.sh           # Build script
├── linux/             # Linux platform implementation
├── freebsd/           # FreeBSD platform implementation
├── win32/             # Windows platform implementation
├── wii/               # Nintendo Wii platform implementation
├── unix/              # Shared Unix code
└── projects/          # Visual Studio project files
```

