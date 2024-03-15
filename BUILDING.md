# Traintastic - Build instructions

For generic project information, see [README.md](README.md).

## Build Traintastic client

### Requirements

- C++ compiler: MSVC, MinGW, GCC or clang.
- CMake 3.9+
- Qt 5.12+ or Qt 6

### Common

Create a build directory:
- From the project root go into the client directory: `cd client`
- Create a build directory: `cmake -E make_directory build`
- Go into the created build directory: `cd build`

### Windows (VS2019, MSVC)

In the *build* directory:
- Configure CMake: `cmake ../ -G "Visual Studio 16 2019" -A x64 -DCMAKE_BUILD_TYPE=Release`
- Build traintastic-client: `cmake --build . --config Release`

### Windows (MinGW)

> NOTE: GCC 8.1 has a bug in `<filesystem>` standard header which prevents compilation
To avoid build errors update to a newer version, e.g. MinGW 11.2.0

In the *build* directory:
- Configure CMake: `cmake ../ -G "MinGW Makefiles" -A x64 -DCMAKE_BUILD_TYPE=Release`
- Build traintastic-client: `cmake --build . --config Release`

#### Debug builds (Client and Server)

For *Debug* builds use `-DCMAKE_BUILD_TYPE=Debug`
Turning off some optimizations might help debugging when stepping inside source code. Set `CMAKE_CXX_FLAGS_DEBUG="-g -Og"`

GDB takes a lot to start debugging because both client and server load many DLL on Windows.
To mitigate that turn off auto loading of shared library symbols BEFORE starting the inferior with GDB command:

`set auto-solib-add off`

If you need to step into some library code (for example Qt libraries in Client) you can load a library by running GDB command:

`sharedlibrary LIBRARY_NAME`
This must be done AFTER inferior has started, symbol loading is delayed until fisrt breakpoint is hit or at `break` command

More info in [GDB Documentation](https://sourceware.org/gdb/onlinedocs/gdb/Files.html)

#### Client only

Qt debugging symbols are distributed separately in `NAME.dll.debug` files.
These files needs to be copied in same directory of the deployed DLL so GDB automatically loads them.
They usually are in `C:\Qt\5.15.2\mingw81_64\lib`

### Linux

In the *build* directory:
- Configure CMake: `cmake ../ -DCMAKE_BUILD_TYPE=Release`
- Build traintastic-client: `cmake --build . --config Release`

### macOS

In the *build* directory:
- Configure CMake: `cmake ../ -DCMAKE_BUILD_TYPE=Release -DQt5_DIR=/path/to/Qt/5.15.2/clang_64/lib/cmake/Qt5`
- Build traintastic-client: `cmake --build . --config Release`


## Building Traintastic server

### Requirements

- C++ compiler: MinGW, GCC or Clang (MSVC doesn't work yet)
- CMake 3.9+
- boost (Linux only)
- liblua5.3 (non Windows only)
- libarchive (non Windows only)
- libsystemd (Linux only)

### Common

Create a build directory:
- From the project root go into the server directory: `cd server`
- Create a build directory: `cmake -E make_directory build`
- Go into the created build directory: `cd build`

### Windows (VS2019, clang)

In the *build* directory:
- Configure CMake: `cmake ../ -G "Visual Studio 16 2019" -A x64 -T ClangCL -DCMAKE_BUILD_TYPE=Release`
- Build traintastic-server: `cmake --build . --config Release --target traintastic-server`

### Windows (MinGW)

> NOTE: GCC 8.1 has a bug in `<filesystem>` standard header which prevents compilation
To avoid build errors update to a newer version, e.g. MinGW 11.2.0

In the *build* directory:
- Configure CMake: `cmake ../ -G "MinGW Makefiles" -A x64 -DCMAKE_BUILD_TYPE=Release`
- Build traintastic-client: `cmake --build . --config Release`

For *Debug builds* see Client section of MinGW

### Linux

Install dependecies, for Ubuntu:
- `sudo apt install libboost-dev libboost-program-options-dev liblua5.3-dev zlib1g-dev libarchive-dev` (required)
- `sudo apt install libsystemd-dev` (optional, for serial port listing)

In the *build* directory:
- Configure CMake: `cmake ../ -DCMAKE_BUILD_TYPE=Release`
- Build traintastic-server: `cmake --build . --config Release --target traintastic-server`

### macOS

In the *build* directory:
- Configure CMake: `cmake ../ -DCMAKE_BUILD_TYPE=Release`
- Build traintastic-server: `cmake --build . --config Release --target traintastic-server`


## Build Traintastic manual

### Requirements

- Python 3.6+ (older versions untested)
- cmarkgfm (`pip3 install cmarkgfm`)

### All platforms

- From the project root go into the manual directory: `cd manual`
- Run the build script: `python3 builddoc.py html-single-page --output-dir build`
