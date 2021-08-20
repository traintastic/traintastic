cmake_minimum_required(VERSION 3.9)
include(../shared/version.cmake)
project(traintastic-server VERSION ${TRAINTASTIC_VERSION} DESCRIPTION "Traintastic server")
include(GNUInstallDirs)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

include(code-coverage)

configure_file(../shared/src/traintastic/version.hpp.in version.hpp)

add_executable(traintastic-server src/main.cpp src/options.hpp)
add_executable(traintastic-server-test test/main.cpp)

add_definitions(
  -DBOOST_ALL_NO_LIB
  -DBOOST_ERROR_CODE_HEADER_ONLY
  -DBOOST_CHRONO_HEADER_ONLY
  -DBOOST_ASIO_HEADER_ONLY
  -DBOOST_SYSTEM_NO_DEPRECATED)

set_target_properties(traintastic-server PROPERTIES CXX_STANDARD 17)
set_target_properties(traintastic-server-test PROPERTIES CXX_STANDARD 17)

target_include_directories(traintastic-server PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}
  ../shared/src/
  ../shared/thirdparty
  thirdparty
  thirdparty/boost)

target_include_directories(traintastic-server-test PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}
  ../shared/src/
  ../shared/thirdparty
  thirdparty
  thirdparty/boost)

file(GLOB SOURCES
  "src/board/*.hpp"
  "src/board/*.cpp"
  "src/board/tile/*.hpp"
  "src/board/tile/*.cpp"
  "src/board/tile/rail/*.hpp"
  "src/board/tile/rail/*.cpp"
  "src/board/tile/rail/signal/*.hpp"
  "src/board/tile/rail/signal/*.cpp"
  "src/board/tile/rail/turnout/*.hpp"
  "src/board/tile/rail/turnout/*.cpp"
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
  "src/hardware/input/list/*.hpp"
  "src/hardware/input/list/*.cpp"
  "src/hardware/input/map/*.hpp"
  "src/hardware/input/map/*.cpp"
  "src/hardware/input/monitor/*.hpp"
  "src/hardware/input/monitor/*.cpp"
  "src/hardware/output/*.hpp"
  "src/hardware/output/*.cpp"
  "src/hardware/output/keyboard/*.hpp"
  "src/hardware/output/keyboard/*.cpp"
  "src/hardware/output/list/*.hpp"
  "src/hardware/output/list/*.cpp"
  "src/hardware/output/map/*.hpp"
  "src/hardware/output/map/*.cpp"
  "src/hardware/protocol/dccplusplus/*.hpp"
  "src/hardware/protocol/dccplusplus/*.cpp"
  "src/hardware/protocol/loconet/*.hpp"
  "src/hardware/protocol/loconet/*.cpp"
  "src/hardware/protocol/xpressnet/*.hpp"
  "src/hardware/protocol/xpressnet/*.cpp"
  "src/hardware/protocol/z21/*.hpp"
  "src/hardware/protocol/z21/*.cpp"
  "src/log/*.hpp"
  "src/log/*.cpp"
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
  "../shared/src/traintastic/locale/locale.cpp"
  "../shared/src/traintastic/utils/standardpaths.cpp"
  "thirdparty/boost/libs/program_options/src/*.cpp")

file(GLOB TEST_SOURCES
  "test/objectcreatedestroy.cpp"
  )

### PLATFORM ###

if(WIN32)
  add_definitions(-D_WIN32_WINNT=0x0601)
  file(GLOB SOURCES_WINDOWS "src/os/windows/*.hpp" "src/os/windows/*.cpp")
  list(APPEND SOURCES ${SOURCES_WINDOWS} src/version.rc ../shared/gfx/appicon.rc)
  target_link_libraries(traintastic-server PRIVATE bcrypt)
  target_link_libraries(traintastic-server-test PRIVATE bcrypt)
endif()

### COMPILER ###

if(MSVC)

else()
  #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -Wshadow -pedantic")

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -pedantic")

  message(STATUS "CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS}")

  #set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
  #set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

  target_link_libraries(traintastic-server PRIVATE pthread)
  target_link_libraries(traintastic-server-test PRIVATE pthread)

  if(NOT APPLE)
    target_link_libraries(traintastic-server PRIVATE stdc++fs)
    target_link_libraries(traintastic-server-test PRIVATE stdc++fs)
  endif()
endif()

### LIBRARIES ###

# libarchive
if(WIN32)
  set(LibArchive_INCLUDE_DIRS "thirdparty/libarchive/include")
  set(LibArchive_LIBRARIES archive)

  add_custom_command(TARGET traintastic-server PRE_LINK
    COMMAND lib "/def:${PROJECT_SOURCE_DIR}/thirdparty/libarchive/bin/archive.def" /out:archive.lib /machine:x64)
  add_custom_command(TARGET traintastic-server-test PRE_LINK
    COMMAND lib "/def:${PROJECT_SOURCE_DIR}/thirdparty/libarchive/bin/archive.def" /out:archive.lib /machine:x64)

  # copy archive.dll to build directory:
  add_custom_command(TARGET traintastic-server POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${PROJECT_SOURCE_DIR}/thirdparty/libarchive/bin/archive.dll" .)
else()
  find_package(LibArchive REQUIRED)
endif()
target_include_directories(traintastic-server PRIVATE ${LibArchive_INCLUDE_DIRS})
target_link_libraries(traintastic-server PRIVATE ${LibArchive_LIBRARIES})
target_include_directories(traintastic-server-test PRIVATE ${LibArchive_INCLUDE_DIRS})
target_link_libraries(traintastic-server-test PRIVATE ${LibArchive_LIBRARIES})

### OPTIONS ###

option(ENABLE_LUA_SCRIPTING "Lua scripting support" ON)
message(STATUS "Lua scripting support: ${ENABLE_LUA_SCRIPTING}")
if(ENABLE_LUA_SCRIPTING)
  if(WIN32)
    set(LUA_INCLUDE_DIR "thirdparty/lua5.3/include")
    set(LUA_LIBRARIES lua53)

    add_custom_command(TARGET traintastic-server PRE_LINK
      COMMAND lib "/def:${PROJECT_SOURCE_DIR}/thirdparty/lua5.3/bin/win64/lua53.def" /out:lua53.lib /machine:x64)
    add_custom_command(TARGET traintastic-server-test PRE_LINK
      COMMAND lib "/def:${PROJECT_SOURCE_DIR}/thirdparty/lua5.3/bin/win64/lua53.def" /out:lua53.lib /machine:x64)

    # copy lua53.dll to build directory, to be able to run the tests:
    add_custom_command(TARGET traintastic-server-test POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy "${PROJECT_SOURCE_DIR}/thirdparty/lua5.3/bin/win64/lua53.dll" .)
  else()
    find_package(Lua 5.3 REQUIRED)
  endif()

  file(GLOB SOURCES_LUA "src/lua/*.hpp" "src/lua/*.cpp")
  list(APPEND SOURCES ${SOURCES_LUA})
  target_include_directories(traintastic-server PRIVATE ${LUA_INCLUDE_DIR})
  target_link_libraries(traintastic-server PRIVATE ${LUA_LIBRARIES})

  file(GLOB TEST_SOURCES_LUA "test/lua/*.cpp")
  list(APPEND TEST_SOURCES ${TEST_SOURCES_LUA})
  target_include_directories(traintastic-server-test PRIVATE ${LUA_INCLUDE_DIR})
  target_link_libraries(traintastic-server-test PRIVATE ${LUA_LIBRARIES})
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
  target_link_libraries(traintastic-server PRIVATE ${USBXPRESSNET_LIB})

  target_include_directories(traintastic-server-test PRIVATE ${USBXPRESSNET_INCLUDE_DIR})
  target_link_libraries(traintastic-server-test PRIVATE ${USBXPRESSNET_LIB})
else()
  list(FILTER SOURCES EXCLUDE REGEX ".*usbxpressnet(interface|controller)\.(hpp|cpp)$")
endif()

### OPTIONS END ###

target_sources(traintastic-server PRIVATE ${SOURCES})
target_sources(traintastic-server-test PRIVATE ${TEST_SOURCES} ${SOURCES})

### CODE COVERAGE ###

target_code_coverage(traintastic-server-test AUTO EXCLUDE "${PROJECT_SOURCE_DIR}/test/*" "${PROJECT_SOURCE_DIR}/thirdparty/*")

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
