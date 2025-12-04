## TODO

- [ ] Move CConArtist to its own package
- [ ] Configuration in JSON loaded at startup
- [ ] GUI to manage servers and view incoming messages

### Completed features

- [x] ImGui as ccode package `cimgui`
- [x] Raylib as ccode package `craylib`
- [x] Glfw as ccode package `cglfw`
- [x] Memory mapped file IO library `cmmio`
- [x] Decoders written as shared libraries (.so/.dylib/.dll) that can be loaded/unloaded at runtime
- [x] File system watcher to monitor decoder folder for changes
- [x] Reuse of many objects to minimize allocations
- [x] UI in a thread running ImmGui
- [x] Main thread is running the event loop with libuv

### Future features

- [ ] Functionality to send TCP and UDP messages from the GUI
- [ ] Support for more protocols (e.g., HTTP, FTP)
