# ConArtist

An application that can run multiple TCP and UDP servers and shows them in a graphical interface.
It will also display incoming messages in real-time and supports a way to decode them using user-defined scripts.

## Packet decoders

Packet decoders are written in C/C++ and compiled as a shared library, you can see an example in `source/cconartist/plugins/example_decoder`.
