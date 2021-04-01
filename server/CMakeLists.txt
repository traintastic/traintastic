cmake_minimum_required(VERSION 3.9)
project(traintastic-server VERSION 0.0.1 DESCRIPTION "Traintastic server")
include(GNUInstallDirs)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

add_executable(traintastic-server src/main.cpp src/options.hpp)
add_executable(traintastic-server-test test/main.cpp)

add_definitions(
  -DVERSION=${PROJECT_VERSION}
  -DBOOST_ALL_NO_LIB
  -DBOOST_ERROR_CODE_HEADER_ONLY
  -DBOOST_CHRONO_HEADER_ONLY
  -DBOOST_ASIO_HEADER_ONLY
  -DBOOST_SYSTEM_NO_DEPRECATED)

set_target_properties(traintastic-server PROPERTIES CXX_STANDARD 17)
set_target_properties(traintastic-server-test PROPERTIES CXX_STANDARD 17)

target_include_directories(traintastic-server PRIVATE
  ../shared/src/
  thirdparty
  thirdparty/boost)

target_include_directories(traintastic-server-test PRIVATE
  ../shared/src/
  thirdparty
  thirdparty/boost)

file(GLOB SOURCES
  "src/board/*.hpp"
  "src/board/*.cpp"
  "src/board/tile/*.hpp"
  "src/board/tile/*.cpp"
  "src/board/tile/rail/*.hpp"
  "src/board/tile/rail/*.cpp"
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
  "src/hardware/output/*.hpp"
  "src/hardware/output/*.cpp"
  "src/hardware/output/map/*.hpp"
  "src/hardware/output/map/*.cpp"
  "src/hardware/protocol/loconet/*.hpp"
  "src/hardware/protocol/loconet/*.cpp"
  "src/hardware/protocol/xpressnet/*.hpp"
  "src/hardware/protocol/xpressnet/*.cpp"
  "src/hardware/protocol/z21/*.hpp"
  "src/hardware/protocol/z21/*.cpp"
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

file(GLOB TEST_SOURCES
  )

### PLATFORM ###

if(WIN32)
  add_definitions(-D_WIN32_WINNT=0x0601)
  target_link_libraries(traintastic-server bcrypt)
  target_link_libraries(traintastic-server-test bcrypt)
endif()

### COMPILER ###

if(MSVC)

else()
  #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -Wshadow -pedantic")

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -pedantic")

  message(STATUS "CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS}")

  #set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
  #set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

  target_link_libraries(traintastic-server pthread stdc++fs)
  target_link_libraries(traintastic-server-test pthread stdc++fs)
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

  file(GLOB TEST_SOURCES_LUA "test/lua/*.cpp")
  list(APPEND TEST_SOURCES ${TEST_SOURCES_LUA})
  target_include_directories(traintastic-server-test PRIVATE ${LUA_INCLUDE_DIR})
  target_link_libraries(traintastic-server-test ${LUA_LIBRARIES})
else()
  add_definitions(-DDISABLE_LUA_SCRIPTING)
endif()

option(USB_XPRESSNET "USB XpressNet interface/controller support" ON)
message(STATUS "USB XpressNet interface/controller support: ${USB_XPRESSNET}")
if(ENABLE_USB_XPRESSNET_INTERFACE)
  #pkg_check_modules(USBXPRESSNET REQUIRED IMPORTED_TARGET usbxpressnet)
  find_path(USBXPRESSNET_INCLUDE_DIR usbxpressnet.h)
  #message(${USBXPRESSNET_INCLUDE_DIR})
  find_library(USBXPRESSNET_LIB usbxpressnet)
  #message(${USBXPRESSNET_LIB})
  #if(NOT USBXPRESSNET_LIB)
  #  message(FATAL_ERROR "usbxpressnet library not found")
  #endif()

  add_definitions(-DUSB_XPRESSNET)

  target_include_directories(traintastic-server PRIVATE ${USBXPRESSNET_INCLUDE_DIR})
  target_link_libraries(traintastic-server ${USBXPRESSNET_LIB})

  target_include_directories(traintastic-server-test PRIVATE ${USBXPRESSNET_INCLUDE_DIR})
  target_link_libraries(traintastic-server-test ${USBXPRESSNET_LIB})
else()
  list(FILTER SOURCES EXCLUDE REGEX ".*usbxpressnet(interface|controller)\.(hpp|cpp)$")
endif()

### OPTIONS END ###

target_sources(traintastic-server PRIVATE ${SOURCES})
target_sources(traintastic-server-test PRIVATE ${TEST_SOURCES} ${SOURCES})

### INSTALL ###

install(TARGETS traintastic-server
    COMPONENT traintastic-server
    RUNTIME DESTINATION "/opt/traintastic/bin/"
)

### TEST ###

include(CTest)
include(Catch)
target_include_directories(traintastic-server-test PRIVATE thirdparty/catch2)
catch_discover_tests(traintastic-server-test)

### Doxygen ###

find_package(Doxygen OPTIONAL_COMPONENTS dot mscgen dia)

if(DOXYGEN_FOUND)
  doxygen_add_docs(doxygen
  ${SOURCES})
    #[filesOrDirs...]
    #[ALL]
    #[USE_STAMP_FILE]
    #[WORKING_DIRECTORY dir]
    #[COMMENT comment])
endif()

### DEB package ###

if(NOT DEBIAN_PACKAGE_VERSION_EXTRA)
  set(DEBIAN_PACKAGE_VERSION_EXTRA "${PROJECT_VERSION_EXTRA}")
endif()

set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}${DEBIAN_PACKAGE_VERSION_EXTRA}")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_DEBIAN_PACKAGE_SECTION "misc")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Reinder Feenstra")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://traintastic.org")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

include(CPack)
