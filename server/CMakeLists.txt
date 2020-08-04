cmake_minimum_required(VERSION 3.9)
project(traintastic-server VERSION 0.0.1 DESCRIPTION "Traintastic server")
include(GNUInstallDirs)

add_executable(traintastic-server src/main.cpp src/options.hpp)

add_definitions(
  -DVERSION=${PROJECT_VERSION}
  -DBOOST_ALL_NO_LIB
  -DBOOST_ERROR_CODE_HEADER_ONLY
  -DBOOST_CHRONO_HEADER_ONLY
  -DBOOST_ASIO_HEADER_ONLY
  -DBOOST_SYSTEM_NO_DEPRECATED)

set_target_properties(traintastic-server PROPERTIES CXX_STANDARD 17)

target_include_directories(traintastic-server PRIVATE
  ../shared/src/
  thirdparty
  thirdparty/boost)

file(GLOB SOURCES
  "src/clock/*.hpp"
  "src/clock/*.cpp"
  "src/core/*.hpp"
  "src/core/*.cpp"
  "src/enum/*.hpp"
  "src/hardware/commandstation/*.hpp"
  "src/hardware/commandstation/*.cpp"
  "src/hardware/controller/*.hpp"
  "src/hardware/controller/*.cpp"
  "src/hardware/decoder/*.hpp"
  "src/hardware/decoder/*.cpp"
  "src/hardware/input/*.hpp"
  "src/hardware/input/*.cpp"
  "src/hardware/protocol/*.hpp"
  "src/hardware/protocol/*.cpp"
  "src/hardware/protocol/loconet/*.hpp"
  "src/hardware/protocol/loconet/*.cpp"
  "src/hardware/protocol/xpressnet/*.hpp"
  "src/hardware/protocol/xpressnet/*.cpp"
  "src/train/*.hpp"
  "src/train/*.cpp"
  "src/vehicle/*.hpp"
  "src/vehicle/*.cpp"
  "src/vehicle/rail/*.hpp"
  "src/vehicle/rail/*.cpp"
  "src/utils/*.hpp"
  "src/utils/*.cpp"
  "src/world/*.hpp"
  "src/world/*.cpp"
  "thirdparty/boost/libs/program_options/src/*.cpp")

### PLATFORM ###

if(WIN32)
  add_definitions(-D_WIN32_WINNT=0x0601)
  target_link_libraries(traintastic-server bcrypt)
endif()

### COMPILER ###

if(MSVC)

else()
  #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -Wshadow -pedantic")

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -pedantic")

  message(STATUS "CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS}")

  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
  set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

  target_link_libraries(traintastic-server pthread stdc++fs)
endif()

### OPTIONS ###

option(ENABLE_LUA_SCRIPTING "Lua scripting support" ON)
message(STATUS "Lua scripting support: ${ENABLE_LUA_SCRIPTING}")
if(ENABLE_LUA_SCRIPTING)
  find_package(Lua 5.3 REQUIRED)
  file(GLOB SOURCES_LUA "src/lua/*.hpp" "src/lua/*.cpp")
  list(APPEND SOURCES ${SOURCES_LUA})
  target_include_directories(traintastic-server PRIVATE ${LUA_INCLUDE_DIR})
  target_link_libraries(traintastic-server ${LUA_LIBRARIES})
else()
  add_definitions(-DDISABLE_LUA_SCRIPTING)
endif()

option(ENABLE_USB_XPRESSNET_INTERFACE "USB XpressNet interface support" ON)
message(STATUS "USB XpressNet interface support: ${ENABLE_USB_XPRESSNET_INTERFACE}")
if(ENABLE_USB_XPRESSNET_INTERFACE)
  #pkg_check_modules(USBXPRESSNET REQUIRED IMPORTED_TARGET usbxpressnet)
  find_path(USBXPRESSNET_INCLUDE_DIR usbxpressnet.h)
  #message(${USBXPRESSNET_INCLUDE_DIR})
  find_library(USBXPRESSNET_LIB usbxpressnet)
  #message(${USBXPRESSNET_LIB})
  #if(NOT USBXPRESSNET_LIB)
  #  message(FATAL_ERROR "usbxpressnet library not found")
  #endif()

  target_include_directories(traintastic-server PRIVATE ${USBXPRESSNET_INCLUDE_DIR})
  target_link_libraries(traintastic-server ${USBXPRESSNET_LIB})
else()
  add_definitions(-DDISABLE_USB_XPRESSNET_INTERFACE)
  list(FILTER SOURCES EXCLUDE REGEX ".*usbxpressnetinterface\.(hpp|cpp)$")
endif()

### OPTIONS END ###

target_sources(traintastic-server PRIVATE ${SOURCES})
