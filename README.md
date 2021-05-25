# Traintastic - Model railway control software

[![Build](https://github.com/traintastic/traintastic/actions/workflows/build.yml/badge.svg)](https://github.com/traintastic/traintastic/actions/workflows/build.yml) ![GitHub](https://img.shields.io/github/license/traintastic/traintastic)


## About The Project
Traintastic is a client/server software application to control a model railway. It is in an early stage of development, it contains very limited functionality.

The project goal is to develop open source software that can control everything in your model railway layout. More information can be found using the links below:

- [Download Traintastic](https://traintastic.org/download)
- [Traintastic Manual](https://traintastic.org/manual)
- [Traintastic hardware support](https://traintastic.org/supported-hardware)
- [Traintastic development roadmap](https://traintastic.org/roadmap)


## Build Traintastic from source

*This section is only for developers, if your just want to use it [download Traintastic](https://traintastic.org/download).* 


### Requirements:

- Client:
  - C++ compiler: MSVC, GCC or Clang
  - Visual Studio 16 2019 (Windows only)
  - CMake 3.9+
  - Qt 5.15+
- Server:
  - C++ compiler: GCC or Clang (MSVC doesn't work yet)
  - Visual Studio 16 2019 (Windows only)
  - CMake
  - liblua5.3 (Linux only)
- Manual:
  - Python 3.6+ (older versions untested)
  - pycmarkgfm (`pip3 install pycmarkgfm`)

Note: When cloning the source from git, git-lfs is required.


### Build Traintastic client

- From the project root go into the client directory: `cd client`
- Create a build directory: `cmake -E make_directory build`
- Go into the created build directory: `cd build`
- Run CMake:
  - Windows: `cmake ../ -G "Visual Studio 16 2019" -A x64 -DCMAKE_BUILD_TYPE=Release`
  - Linux: `cmake ../ -DCMAKE_BUILD_TYPE=Release`
- Build traintastic-client: `cmake --build . --config Release`


### Build Traintastic server

- From the project root go into the server directory: `cd server`
- Create a build directory: `cmake -E make_directory build`
- Go into the created build directory: `cd build`
- Configure CMake:
  - Windows: `cmake ../ -G "Visual Studio 16 2019" -A x64 -T ClangCL -DCMAKE_BUILD_TYPE=Release -DUSB_XPRESSNET=OFF`
  - Linux: `cmake ../ -DCMAKE_BUILD_TYPE=Release -DUSB_XPRESSNET=OFF`
- Build traintastic-server: `cmake --build . --config Release --target traintastic-server`


### Build Traintastic manual

1. From the project root go into the manual directory: `cd manual`
2. Run the build script: `python3 builddoc.py html-single-page --output-dir build`
