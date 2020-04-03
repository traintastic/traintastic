cmake_minimum_required(VERSION 3.9)
project(traintastic-server VERSION 0.1.0 DESCRIPTION "Traintastic server")
include(GNUInstallDirs)

#find_package(PkgConfig REQUIRED)
find_package(Lua 5.3 REQUIRED)
#pkg_check_modules(USBXPRESSNET REQUIRED IMPORTED_TARGET usbxpressnet)

find_path(USBXPRESSNET_INCLUDE_DIR usbxpressnet.h)
#message(${USBXPRESSNET_INCLUDE_DIR})
find_library(USBXPRESSNET_LIB usbxpressnet)
#message(${USBXPRESSNET_LIB})
#if(NOT USBXPRESSNET_LIB)
#  message(FATAL_ERROR "usbxpressnet library not found")
#endif()

set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")


file(GLOB SOURCES
  "src/*.hpp"
  "src/*.cpp"
  "src/clock/*.hpp"
  "src/clock/*.cpp"
  "src/core/*.hpp"
  "src/core/*.cpp"
  "src/enum/*.hpp"
  "src/hardware/commandstation/*.hpp"
  "src/hardware/commandstation/*.cpp"
  "src/hardware/decoder/*.hpp"
  "src/hardware/decoder/*.cpp"
  "src/hardware/input/*.hpp"
  "src/hardware/input/*.cpp"
  "src/hardware/protocol/*.hpp"
  "src/hardware/protocol/*.cpp"
  "src/lua/*.hpp"
  "src/lua/*.cpp"
  "src/utils/*.hpp"
  "thirdparty/boost/libs/program_options/src/*.cpp")

add_executable(traintastic-server ${SOURCES})

add_definitions(
  -DVERSION=${PROJECT_VERSION}
  -DBOOST_ALL_NO_LIB
  -DBOOST_ERROR_CODE_HEADER_ONLY
  -DBOOST_CHRONO_HEADER_ONLY
  -DBOOST_ASIO_HEADER_ONLY
  -DBOOST_SYSTEM_NO_DEPRECATED)

target_include_directories(traintastic-server PRIVATE
  ../shared/src/
  thirdparty
  thirdparty/boost
  ${LUA_INCLUDE_DIR}
  ${USBXPRESSNET_INCLUDE_DIR})

target_link_libraries(traintastic-server
  pthread
  stdc++fs
  ${LUA_LIBRARIES}
  ${USBXPRESSNET_LIB})
#  PkgConfig::USBXPRESSNET)

set_target_properties(traintastic-server PROPERTIES CXX_STANDARD 17)
