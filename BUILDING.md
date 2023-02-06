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

*TODO*

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

*TODO*

### Linux

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
