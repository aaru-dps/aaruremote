Aaru Remote
===========

Aaru Remote lets [Aaru](https://github.com/aaru-dps/Aaru) use drives that are connected to another computer.

You run the small Aaru Remote server on the machine that has the drive, and run Aaru anywhere else on your network.
Aaru sends the commands over the network, Aaru Remote passes them to the drive, and sends the results back. Dumping,
getting device information and every other Aaru command that works with a device work the same way as with a local
drive.

This is useful when:

- The drive only works on an old computer or operating system that cannot run a modern Aaru.
- The drive is attached to a machine where .NET is not available, such as FreeBSD, an old Mac, or a Nintendo Wii.
- You prefer to keep Aaru on your main computer while the drive sits on a dedicated machine.

Aaru Remote is a small C program with no runtime to install, so it runs on systems as old as Linux 2.6, Windows XP and
Mac OS X 10.0.

Quick start
===========

1. **Get Aaru Remote on the machine with the drive.** Build it as described in [Building from source](#building-from-source).
2. **Start it as root or Administrator:**

   ```
   sudo ./aaruremote
   ```

   It prints the IP addresses it is listening on, for example `192.168.1.10 port 6666`.
3. **Check the connection from the machine running Aaru:**

   ```
   aaru remote 192.168.1.10
   ```

4. **List the remote drives:**

   ```
   aaru device list 192.168.1.10
   ```

5. **Use a drive** by giving Aaru a device path of the form `aaru://<IP address>/<device path>`:

   ```
   aaru device info aaru://192.168.1.10/dev/sr0
   aaru media dump aaru://192.168.1.10/dev/sr0 disc.aaruf
   ```

Running the server
==================

Aaru Remote listens for one Aaru connection at a time on TCP port **6666**. When a session ends it waits for the next
one, so you can leave it running.

**Run it with administrator rights.** Sending raw commands to drives needs root on Linux, FreeBSD and macOS, and
Administrator on Windows. Without them only some SCSI devices work.

While it is waiting for a connection:

| Platform              | List the local drives | Quit                |
|-----------------------|-----------------------|---------------------|
| Linux, FreeBSD, macOS | Press **D**           | Ctrl+C              |
| Windows               | Press **D**           | Ctrl+C              |
| Nintendo Wii          | Press **A**           | Press **Home**      |

If Aaru cannot connect, make sure the firewall on the server machine allows incoming TCP connections on port 6666.

Device paths
============

The part after the IP address in an `aaru://` path is the device path on the server. The easiest way to find it is
`aaru device list <IP address>`, which prints the complete `aaru://` paths, or pressing **D** on the server.

| Server OS    | Typical device paths                                                                       |
|--------------|--------------------------------------------------------------------------------------------|
| Linux        | `/dev/sr0` (optical), `/dev/sda` (disks), `/dev/sg0` (SCSI generic), `/dev/mmcblk0` (SD/MMC) |
| FreeBSD      | `/dev/pass0`, or `/dev/ad0` and `/dev/acd0` on FreeBSD 8 and older                         |
| macOS        | `/dev/rdisk2`                                                                              |
| Windows      | `\\.\PhysicalDrive0`, `\\.\CdRom0`, `\\.\Tape0`                                            |
| Nintendo Wii | `/dev/flash` (NAND), `/dev/sdio/slot0` (SD card)                                           |

For example `aaru://192.168.1.10/dev/sr0`, or `aaru://192.168.1.20/\\.\CdRom0` for a Windows server. Remember to quote
Windows paths in your shell.

Security
========

Aaru Remote has **no authentication and no encryption**. Anyone who can reach port 6666 can send any command to any
drive on that machine, including commands that write to or erase it, with the administrator rights the server runs
with.

- Only run it on a network you trust, never on a machine reachable from the internet.
- Use a firewall to allow port 6666 only from the computer running Aaru.
- Stop it when you are done.

Supported platforms
===================

|              | Minimum OS<sup>*1</sup> | SCSI            | CHS ATA                  | 28-bit LBA ATA           | 48-bit LBA ATA           | Secure Digital | MultiMediaCard | USB                  | FireWire             | PCMCIA           | Special<sup>*2</sup> |
|--------------|-------------------------|-----------------|--------------------------|--------------------------|--------------------------|----------------|----------------|----------------------|----------------------|------------------|----------------------|
| FreeBSD      | 5.5<sup>*8</sup>        | Yes             | Yes                      | Yes                      | Yes                      | Not yet        | Not yet        | Not yet<sup>*4</sup> | Not yet<sup>*4</sup> | No<sup>*5</sup>  |                      |
| Linux        | 2.6                     | Yes             | Yes                      | Yes                      | Yes                      | Yes            | Yes            | Yes                  | Yes                  | Yes<sup>*6</sup> |                      |
| macOS        | 10.0                    | Yes             | Best effort<sup>*7</sup> | Best effort<sup>*7</sup> | Best effort<sup>*7</sup> | Not yet        | Not yet        | Yes                  | Yes                  | No               |                      |
| Nintendo Wii | 4.3                     | No<sup>*3</sup> | No<sup>*3</sup>          | No<sup>*3</sup>          | No<sup>*3</sup>          | Not yet        | Not yet        | Not yet              | No<sup>*3</sup>      | No<sup>*3</sup>  | Not yet              |
| Windows NT   | XP                      | Yes             | Yes                      | Yes                      | Yes                      | Yes            | Yes            | Yes                  | Not yet<sup>*4</sup> | No<sup>*5</sup>  |                      |

1. Minimum operating system version where aaruremote has been tested. May work on early version.
2. Special storage media only available on that environment.
3. Hardware not available or supported by the operating system.
4. As SCSI device, not possible to retrieve special data.
5. As ATA device, not possible to retrieve special data.
6. Only ATA devices, not linear memory devices.
7. ATA passthrough on macOS is attempted through the IOKit storage stack and SAT-compatible bridges, but modern macOS may restrict raw low-level access depending on the device and OS version.
8. Tested on FreeBSD 5.5, 8.4 and 12.4. Before FreeBSD 9, ATA and ATAPI devices are attached to the legacy ata(4) driver instead of CAM, which only allows LBA28 commands and does not return the status and error registers. ATAPI drives work through ata(4) from FreeBSD 6, and need a kernel built with `atapicam` on FreeBSD 5.

Troubleshooting
===============

- **Aaru cannot connect:** check the IP address printed by the server, that both machines are on the same network,
  and that port 6666 is allowed through the server's firewall.
- **"Error 98 binding socket" when starting:** another program, or a previous Aaru Remote that just exited, is still
  using port 6666. Wait a minute and start it again.
- **A drive is missing from the list, or commands fail:** make sure the server runs as root or Administrator.
- **The connection closes unexpectedly:** the server and Aaru must speak the same protocol. Update both to recent
  versions.

Building from source
====================

You need a C compiler. Aaru Remote is written in C90, so very old compilers work too.

The resulting program is called `aaruremote` (`aaruremote.exe` on Windows). Its version is `1.0-<git commit>` when built
from a git checkout, or `1.0-unknown` otherwise.

With CMake
----------

CMake 2.8 or newer builds every supported platform:

```
cmake .
make
```

On Linux, install the libudev development files first, if available, for better device listing.

Without CMake
-------------

For systems where CMake is not available, there are ready-made makefiles in `projects/makefile`. They build into a
`build` directory next to the makefile, and support `make install` (with `PREFIX` and `DESTDIR`) and `make clean`.

| System  | Directory                    | Command |
|---------|------------------------------|---------|
| Linux   | `projects/makefile/linux`    | `make`  |
| FreeBSD | `projects/makefile/freebsd`  | `make`  |
| macOS   | `projects/makefile/darwin`   | `make`  |

```
cd projects/makefile/linux
make
sudo make install
```

By default, the binaries are built for the oldest CPU of each architecture, so they run on any machine of that
architecture. Pass `BASELINE_ISA=no` to use the compiler's defaults instead. On Linux, `UDEV=no` builds without libudev.

Windows
-------

Open the Visual Studio solution for your version from `projects/msvc2005`, `projects/msvc2017` or `projects/msvc2019`,
or use CMake.

Nintendo Wii
------------

Install [devkitPro](https://devkitpro.org/) with devkitPPC and libogc, then run `./build.sh`, which also builds the Wii
version when devkitPro is found. Copy the result to your SD card and start it from the Homebrew Channel.

For developers
==============

- The protocol, packet structures and the functions each platform must implement are in `aaruremote.h`.
- `worker.c` contains the network loop and packet handling shared by all platforms. Each platform lives in its own
  directory (`linux`, `freebsd`, `macos`, `win32`, `wii`), with code shared by Unix-like systems in `unix`.
- [AGENTS.md](AGENTS.md) describes the architecture, coding conventions and how to add a new platform.

Planned work:

- Support PSP
- Support Wii
- Support Wii U
- Support connections through serial port
- Timing under Linux

License
=======

Aaru Remote is licensed under the [GNU General Public License v3](LICENSE).
