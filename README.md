DiscImageChef Remote
====================

The DiscImageChef Remote is a slim miniature application designed to receive device commands from a remote [DiscImageChef](https://github.com/discimagechef/DiscImageChef)
instance, sends it to a local device, and returns the data to the instance.

The main motivation for this is the desire to update DiscImageChef to the latest and greatest features of .NET and C#.
This creates a problem, as some people have old devices that do not work in modern Linux distributions.

This remote will be supported in older versions of Linux, and will in future versions be supported in FreeBSD, Windows, and possibly
network-enabled game consoles (like PSP, Wii, etc).

While some people will suggest porting the whole DiscImageChef to C or C++, that won't happen, and for the only situation that
would be needed (accessing devices where C# does not run) this slim is more than enough.

The usage is very simple, just run the remote and it will listen for a connection over TCP/IP in port 6666, and print you
the available IPs. Running as non-root user only works with some SCSI devices, so better run as root.

On the other side, you can use the DiscImageChef with the *remote* command and one of those IP addresses to test the
connection. Similarly using the IP address as an argument for the *list-devices* command will list the devices available
remotely.

All commands that support devices are supported, with a URI with the following schema:
`dic://<IP ADDRESS>/<DEVICE PATH>`.

TODO
====
- More buffer overflow guards
- Support FreeBSD
- Support Windows (Windows XP at the lowest level)
- Support PSP
- Support Wii
- Support Wii U
- Support connections thru serial port
- Timing under Linux

Licensed under the [GPLv3 license](LICENSE.md).
