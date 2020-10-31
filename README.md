Aaru Remote
====================

The Aaru Remote is a slim miniature application designed to receive device commands from a remote [Aaru](https://github.com/aaru-dps/Aaru)
instance, sends it to a local device, and returns the data to the instance.

The main motivation for this is the desire to update Aaru to the latest and greatest features of .NET and C#.
This creates a problem, as some people have old devices that do not work in modern Linux distributions.

This remote will be supported in older versions of Linux, and will in future versions be supported in FreeBSD, Windows, and possibly
network-enabled game consoles (like PSP, Wii, etc).

While some people will suggest porting the whole Aaru to C or C++, that won't happen, and for the only situation that
would be needed (accessing devices where C# does not run) this slim is more than enough.

The usage is very simple, just run the remote and it will listen for a connection over TCP/IP in port 6666, and print you
the available IPs. Running as non-root user only works with some SCSI devices, so better run as root.

On the other side, you can use the Aaru with the *remote* command and one of those IP addresses to test the
connection. Similarly using the IP address as an argument for the *list-devices* command will list the devices available
remotely.

All commands that support devices are supported, with a URI with the following schema:
`aaru://<IP ADDRESS>/<DEVICE PATH>`.

Feature matrix
==============
|            |Minimum OS<sup>*1</sup>|     SCSI      |CHS ATA        |28-bit LBA ATA |48-bit LBA ATA |Secure Digital|MultiMediaCard|USB                 |FireWire            |PCMCIA          |Special<sup>*2</sup>|
|------------|-----------------------|---------------|---------------|---------------|---------------|--------------|--------------|--------------------|--------------------|----------------|-------|
|FreeBSD     | 12                    |Yes            |Yes            |Yes            |Yes            |Not yet       |Not yet       |Not yet<sup>*4</sup>|Not yet<sup>*4</sup>|No<sup>*5</sup> ||
|Linux       |2.6                    |Yes            |Yes            |Yes            |Yes            |Yes           |Yes           |Yes                 |Yes                 |Yes<sup>*6</sup>||
|Nintendo Wii|4.3                    |No<sup>*3</sup>|No<sup>*3</sup>|No<sup>*3</sup>|No<sup>*3</sup>|Not yet       |Not yet       |Not yet             |No<sup>*3</sup>     |No<sup>*3</sup> |Not yet|
|Windows NT  | XP                    |Yes            |Yes            |Yes            |Yes            |Yes           |Yes           |Yes                 |Not yet<sup>*4</sup>|No<sup>*5</sup> ||

1. Minimum operating system version where aaruremote has been tested. May work on early version.
2. Special storage media only available on that environment.
3. Hardware not available or supported by the operating system.
4. As SCSI device, not possible to retrieve special data.
5. As ATA device, not possible to retrieve special data.
6. Only ATA devices, not linear memory devices.

TODO
====
- More buffer overflow guards
- Support PSP
- Support Wii
- Support Wii U
- Support connections thru serial port
- Timing under Linux

Licensed under the [GPLv3 license](LICENSE.md).
