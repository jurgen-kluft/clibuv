# Compiling a plugin

Plugins for CConArtist are compiled as dynamic libraries. Below is an example command to compile a simple plugin on macOS using `clang`. Adjust the command as necessary for your operating system and compiler.

Plugins can be updated and recompiled without needing to recompile the entire CConArtist application, allowing for easy development and testing of new functionality.

When `cconartist` is running and a plugin is added or updated, it is loaded dynamically to extend the application's capabilities.

## Example: A 'sensorpacket' plugin

##
        clang -Wall -O2 -fPIC -dynamiclib sensorpacket.cpp -o sensorpacket.dylib

