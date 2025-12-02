# Compiling a plugin

Plugins for CConArtist are compiled as dynamic libraries. Below is an example command to compile a simple plugin on macOS using `clang`. Adjust the command as necessary for your operating system and compiler.

Plugins can be updated and recompiled without needing to recompile the entire CConArtist application, allowing for easy development and testing of new functionality.

When `cconartist` is running and a plugin is added or updated, it is loaded dynamically to extend the application's capabilities.

## Example: A sensorpacket plugin

The following command compiles a simple `sensorpacket` plugin into a dynamic library named `sensorpacket.dylib`.

##
        clang -Wall -O2 -fPIC -dynamiclib sensorpacket.cpp -o sensorpacket.dylib


## Example: A GeekOpen (JSON) plugin

GeekOpen devices (wall panel light switches, etc..) can be configured to send JSON formatted data packets to a TCP server (IP and Port). The following command compiles a simple `geekopen` plugin into a dynamic library named `geekopen.dylib`.

##
        clang -Wall -O2 -fPIC -dynamiclib geekopen.cpp -o geekopen.dylib

## Notes

- Ensure that you have the necessary development tools and libraries installed for compiling dynamic libraries on your system.
- The `-fPIC` flag is used to generate position-independent code, which is required for shared libraries.
- The `-dynamiclib` flag tells the compiler to create a dynamic library
- The filename will be used as the plugin name when loading it in CConArtist and your config can specify for each server
  which plugins to use to decode incoming data.
