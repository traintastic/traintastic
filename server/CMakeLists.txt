cmake_minimum_required(VERSION 3.9)
include(../shared/traintastic.cmake)
project(traintastic-server VERSION ${TRAINTASTIC_VERSION} DESCRIPTION "Traintastic server")
include(GNUInstallDirs)
include(../shared/translations/traintastic-lang.cmake)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

include(code-coverage)

configure_file(../shared/src/traintastic/version.hpp.in version.hpp)

if(MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Werror)
endif()

if(ENABLE_CLANG_TIDY)
  set(CMAKE_CXX_CLANG_TIDY clang-tidy -checks=-*,readability-*,-readability-braces-around-statements,-readability-implicit-bool-conversion,-readability-magic-numbers)
endif()

add_executable(traintastic-server src/main.cpp src/options.hpp ${TRANSLATION_FILES})
add_executable(traintastic-server-test test/main.cpp ${TRANSLATION_FILES})

target_compile_definitions(traintastic-server-test PRIVATE -DTRAINTASTIC_TEST)

set_target_properties(traintastic-server PROPERTIES CXX_STANDARD 17)
set_target_properties(traintastic-server-test PROPERTIES CXX_STANDARD 17)

target_include_directories(traintastic-server PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}
  ../shared/src)

target_include_directories(traintastic-server SYSTEM PRIVATE
  ../shared/thirdparty
  thirdparty)

target_include_directories(traintastic-server-test PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}
  ../shared/src)

target_include_directories(traintastic-server-test SYSTEM PRIVATE
  ../shared/thirdparty
  thirdparty)

file(GLOB SOURCES
  "src/board/*.hpp"
  "src/board/*.cpp"
  "src/board/list/*.hpp"
  "src/board/list/*.cpp"
  "src/board/map/*.hpp"
  "src/board/map/*.cpp"
  "src/board/nx/*.hpp"
  "src/board/nx/*.cpp"
  "src/board/tile/*.hpp"
  "src/board/tile/*.cpp"
  "src/board/tile/misc/*.hpp"
  "src/board/tile/misc/*.cpp"
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
  "src/hardware/decoder/*.hpp"
  "src/hardware/decoder/*.cpp"
  "src/hardware/decoder/list/*.hpp"
  "src/hardware/decoder/list/*.cpp"
  "src/hardware/identification/*.hpp"
  "src/hardware/identification/*.cpp"
  "src/hardware/identification/list/*.hpp"
  "src/hardware/identification/list/*.cpp"
  "src/hardware/input/*.hpp"
  "src/hardware/input/*.cpp"
  "src/hardware/input/list/*.hpp"
  "src/hardware/input/list/*.cpp"
  "src/hardware/input/map/*.hpp"
  "src/hardware/input/map/*.cpp"
  "src/hardware/input/monitor/*.hpp"
  "src/hardware/input/monitor/*.cpp"
  "src/hardware/interface/*.hpp"
  "src/hardware/interface/*.cpp"
  "src/hardware/interface/marklincan/*.hpp"
  "src/hardware/interface/marklincan/*.cpp"
  "src/hardware/output/*.hpp"
  "src/hardware/output/*.cpp"
  "src/hardware/output/keyboard/*.hpp"
  "src/hardware/output/keyboard/*.cpp"
  "src/hardware/output/list/*.hpp"
  "src/hardware/output/list/*.cpp"
  "src/hardware/output/map/*.hpp"
  "src/hardware/output/map/*.cpp"
  "src/hardware/programming/lncv/*.hpp"
  "src/hardware/programming/lncv/*.cpp"
  "src/hardware/protocol/*.hpp"
  "src/hardware/protocol/*.cpp"
  "src/hardware/protocol/dccplusplus/*.hpp"
  "src/hardware/protocol/dccplusplus/*.cpp"
  "src/hardware/protocol/dccplusplus/iohandler/*.hpp"
  "src/hardware/protocol/dccplusplus/iohandler/*.cpp"
  "src/hardware/protocol/ecos/*.hpp"
  "src/hardware/protocol/ecos/*.cpp"
  "src/hardware/protocol/ecos/iohandler/*.hpp"
  "src/hardware/protocol/ecos/iohandler/*.cpp"
  "src/hardware/protocol/ecos/object/*.hpp"
  "src/hardware/protocol/ecos/object/*.cpp"
  "src/hardware/protocol/loconet/*.hpp"
  "src/hardware/protocol/loconet/*.cpp"
  "src/hardware/protocol/loconet/message/*.hpp"
  "src/hardware/protocol/loconet/message/*.cpp"
  "src/hardware/protocol/loconet/message/uhlenbrock/*.hpp"
  "src/hardware/protocol/loconet/message/uhlenbrock/*.cpp"
  "src/hardware/protocol/loconet/iohandler/*.hpp"
  "src/hardware/protocol/loconet/iohandler/*.cpp"
  "src/hardware/protocol/marklincan/*.hpp"
  "src/hardware/protocol/marklincan/*.cpp"
  "src/hardware/protocol/marklincan/iohandler/*.hpp"
  "src/hardware/protocol/marklincan/iohandler/*.cpp"
  "src/hardware/protocol/marklincan/message/*.hpp"
  "src/hardware/protocol/marklincan/message/*.cpp"
  "src/hardware/protocol/traintasticdiy/*.hpp"
  "src/hardware/protocol/traintasticdiy/*.cpp"
  "src/hardware/protocol/traintasticdiy/iohandler/*.hpp"
  "src/hardware/protocol/traintasticdiy/iohandler/*.cpp"
  "src/hardware/protocol/withrottle/*.hpp"
  "src/hardware/protocol/withrottle/*.cpp"
  "src/hardware/protocol/withrottle/iohandler/*.hpp"
  "src/hardware/protocol/withrottle/iohandler/*.cpp"
  "src/hardware/protocol/xpressnet/*.hpp"
  "src/hardware/protocol/xpressnet/*.cpp"
  "src/hardware/protocol/xpressnet/iohandler/*.hpp"
  "src/hardware/protocol/xpressnet/iohandler/*.cpp"
  "src/hardware/protocol/z21/*.hpp"
  "src/hardware/protocol/z21/*.cpp"
  "src/hardware/protocol/z21/iohandler/*.hpp"
  "src/hardware/protocol/z21/iohandler/*.cpp"
  "src/hardware/throttle/*.hpp"
  "src/hardware/throttle/*.cpp"
  "src/hardware/throttle/list/*.hpp"
  "src/hardware/throttle/list/*.cpp"
  "src/log/*.hpp"
  "src/log/*.cpp"
  "src/lua/*.hpp"
  "src/lua/*.cpp"
  "src/lua/object/*.hpp"
  "src/lua/object/*.cpp"
  "src/network/*.hpp"
  "src/network/*.cpp"
  "src/os/*.hpp"
  "src/os/*.cpp"
  "src/pcap/*.hpp"
  "src/pcap/*.cpp"
  "src/status/*.hpp"
  "src/status/*.cpp"
  "src/train/*.hpp"
  "src/train/*.cpp"
  "src/traintastic/*.hpp"
  "src/traintastic/*.cpp"
  "src/vehicle/*.hpp"
  "src/vehicle/*.cpp"
  "src/vehicle/rail/*.hpp"
  "src/vehicle/rail/*.cpp"
  "src/utils/*.hpp"
  "src/utils/*.cpp"
  "src/world/*.hpp"
  "src/world/*.cpp"
  "../shared/src/traintastic/locale/locale.cpp"
  "../shared/src/traintastic/utils/standardpaths.cpp")

file(GLOB TEST_SOURCES
  "test/board/*.cpp"
  "test/hardware/*.cpp"
  "test/lua/*.cpp"
  "test/lua/script/*.cpp"
  "test/train/*.cpp"
  "test/objectcreatedestroy.cpp"
  )

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DENABLE_LOG_DEBUG")

### PLATFORM ###

if(UNIX AND NOT APPLE)
  set(LINUX TRUE)
  list(APPEND SOURCES "src/os/linux/setbaudrate.hpp" "src/os/linux/setbaudrate.cpp")
endif()

if(UNIX)
  file(GLOB SOURCES_UNIX "src/os/unix/*.hpp" "src/os/unix/*.cpp")
  list(APPEND SOURCES ${SOURCES_UNIX})
endif()

if(LINUX)
  find_package(PkgConfig REQUIRED)

  # libsystemd:
  pkg_check_modules(LIBSYSTEMD REQUIRED IMPORTED_TARGET libsystemd)
  if(${LIBSYSTEMD_FOUND})
    add_definitions(-DHAS_LIBSYSTEMD)
    list(APPEND SOURCES "src/os/linux/serialportlistimplsystemd.hpp" "src/os/linux/serialportlistimplsystemd.cpp")
    target_link_libraries(traintastic-server PRIVATE PkgConfig::LIBSYSTEMD)
    target_link_libraries(traintastic-server-test PRIVATE PkgConfig::LIBSYSTEMD)
  endif()
else()
  # socket CAN is only available on linux:
  file(GLOB SOCKET_CAN_SOURCES "src/hardware/protocol/marklincan/iohandler/socketcaniohandler.*")
  list(REMOVE_ITEM SOURCES ${SOCKET_CAN_SOURCES})
endif()

if(WIN32)
  add_definitions(-D_WIN32_WINNT=0x0601)
  file(GLOB SOURCES_WINDOWS "src/os/windows/*.hpp" "src/os/windows/*.cpp")
  list(APPEND SOURCES ${SOURCES_WINDOWS} src/version.rc ../shared/gfx/appicon.rc)
  target_link_libraries(traintastic-server PRIVATE bcrypt setupapi)
  target_link_libraries(traintastic-server-test PRIVATE bcrypt setupapi)
endif()

### COMPILER ###

if(MSVC)

else()
  ### Enable ASAN and UBSAN
  #set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined")

  ### Use the LLVM lld linker (faster than standard linker)
  #set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld")

  target_link_libraries(traintastic-server PRIVATE pthread)
  target_link_libraries(traintastic-server-test PRIVATE pthread)

  if(NOT APPLE)
    target_link_libraries(traintastic-server PRIVATE stdc++fs)
    target_link_libraries(traintastic-server-test PRIVATE stdc++fs)
  endif()
endif()

### LIBRARIES ###

if(WIN32 AND NOT MSVC)
  # On MinGW ws2_32.dll needs explicit linking
  # Also mswsock.dll is needed for AcceptEx() used by Boost.Asio
  target_link_libraries(traintastic-server PRIVATE ws2_32 mswsock)
  target_link_libraries(traintastic-server-test PRIVATE ws2_32 mswsock)
endif()

# boost
if(LINUX)
  find_package(Boost 1.71 REQUIRED COMPONENTS program_options)

  target_include_directories(traintastic-server SYSTEM PRIVATE ${Boost_INCLUDE_DIRS})
  target_link_libraries(traintastic-server PRIVATE ${Boost_LIBRARIES})
  target_include_directories(traintastic-server-test SYSTEM PRIVATE ${Boost_INCLUDE_DIRS})
  target_link_libraries(traintastic-server-test PRIVATE ${Boost_LIBRARIES})
else()
  add_definitions(
    -DBOOST_ALL_NO_LIB
    -DBOOST_ERROR_CODE_HEADER_ONLY
    -DBOOST_CHRONO_HEADER_ONLY
    -DBOOST_ASIO_HEADER_ONLY
    -DBOOST_SYSTEM_NO_DEPRECATED)

  if(NOT MSVC)
    set_source_files_properties(
        thirdparty/boost/libs/program_options/src/cmdline.cpp
        thirdparty/boost/libs/program_options/src/config_file.cpp
        thirdparty/boost/libs/program_options/src/convert.cpp
        thirdparty/boost/libs/program_options/src/options_description.cpp
        thirdparty/boost/libs/program_options/src/parsers.cpp
        thirdparty/boost/libs/program_options/src/positional_options.cpp
        thirdparty/boost/libs/program_options/src/split.cpp
        thirdparty/boost/libs/program_options/src/utf8_codecvt_facet.cpp
        thirdparty/boost/libs/program_options/src/value_semantic.cpp
        thirdparty/boost/libs/program_options/src/variables_map.cpp
        thirdparty/boost/libs/program_options/src/winmain.cpp
      PROPERTIES
        COMPILE_FLAGS -Wno-shadow)
  endif()

  target_include_directories(traintastic-server SYSTEM PRIVATE thirdparty/boost)
  target_include_directories(traintastic-server-test SYSTEM PRIVATE thirdparty/boost)

  file(GLOB SOURCES_BOOST "thirdparty/boost/libs/program_options/src/*.cpp")
  list(APPEND SOURCES ${SOURCES_BOOST})
endif()

# zlib
if(WIN32)
  set(ZLIB_INCLUDE_DIRS "thirdparty/zlib/include")

  if(MSVC)
      set(ZLIB_LIBRARIES zlib1)
      add_custom_command(TARGET traintastic-server PRE_LINK
        COMMAND lib "/def:${PROJECT_SOURCE_DIR}/thirdparty/zlib/bin/zlib1.def" /out:zlib1.lib /machine:x64)
      add_custom_command(TARGET traintastic-server-test PRE_LINK
        COMMAND lib "/def:${PROJECT_SOURCE_DIR}/thirdparty/zlib/bin/zlib1.def" /out:zlib1.lib /machine:x64)
  else()
      # MinGW can directly link .dll without import lib
      set(ZLIB_LIBRARIES "${PROJECT_SOURCE_DIR}/thirdparty/zlib/bin/zlib1.dll")
  endif()

  # copy zlib1.dll to build directory:
  add_custom_command(TARGET traintastic-server POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${PROJECT_SOURCE_DIR}/thirdparty/zlib/bin/zlib1.dll" .)
else()
  find_package(ZLIB REQUIRED)
endif()
target_include_directories(traintastic-server PRIVATE ${ZLIB_INCLUDE_DIRS})
target_link_libraries(traintastic-server PRIVATE ${ZLIB_LIBRARIES})
target_include_directories(traintastic-server-test PRIVATE ${ZLIB_INCLUDE_DIRS})
target_link_libraries(traintastic-server-test PRIVATE ${ZLIB_LIBRARIES})

# libarchive
if(WIN32)
  set(LibArchive_INCLUDE_DIRS "thirdparty/libarchive/include")

  if(MSVC)
      set(LibArchive_LIBRARIES archive)
      add_custom_command(TARGET traintastic-server PRE_LINK
        COMMAND lib "/def:${PROJECT_SOURCE_DIR}/thirdparty/libarchive/bin/archive.def" /out:archive.lib /machine:x64)
      add_custom_command(TARGET traintastic-server-test PRE_LINK
        COMMAND lib "/def:${PROJECT_SOURCE_DIR}/thirdparty/libarchive/bin/archive.def" /out:archive.lib /machine:x64)
  else()
      # MinGW can directly link .dll without import lib
      set(LibArchive_LIBRARIES "${PROJECT_SOURCE_DIR}/thirdparty/libarchive/bin/archive.dll")
  endif()

  # copy archive.dll to build directory:
  add_custom_command(TARGET traintastic-server POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${PROJECT_SOURCE_DIR}/thirdparty/libarchive/bin/archive.dll" .)
elseif(APPLE)
  set(LibArchive_INCLUDE_DIRS "/usr/local/opt/libarchive/include")
  set(LibArchive_LIBRARIES "/usr/local/opt/libarchive/lib/libarchive.dylib")
else()
  find_package(LibArchive REQUIRED)
endif()
target_include_directories(traintastic-server PRIVATE ${LibArchive_INCLUDE_DIRS})
target_link_libraries(traintastic-server PRIVATE ${LibArchive_LIBRARIES})
target_include_directories(traintastic-server-test PRIVATE ${LibArchive_INCLUDE_DIRS})
target_link_libraries(traintastic-server-test PRIVATE ${LibArchive_LIBRARIES})

# liblua5.3
if(WIN32)
  add_definitions(-DLUA_BUILD_AS_DLL)
  set(LUA_INCLUDE_DIR "thirdparty/lua5.3/include")

  if(MSVC)
      set(LUA_LIBRARIES lua53)
      add_custom_command(TARGET traintastic-server PRE_LINK
        COMMAND lib "/def:${PROJECT_SOURCE_DIR}/thirdparty/lua5.3/bin/win64/lua53.def" /out:lua53.lib /machine:x64)
      add_custom_command(TARGET traintastic-server-test PRE_LINK
        COMMAND lib "/def:${PROJECT_SOURCE_DIR}/thirdparty/lua5.3/bin/win64/lua53.def" /out:lua53.lib /machine:x64)
  else()
      # MinGW can directly link .dll without import lib
      set(LUA_LIBRARIES "${PROJECT_SOURCE_DIR}/thirdparty/lua5.3/bin/win64/lua53.dll")
  endif()


  # copy lua53.dll to build directory, to be able to run the tests:
  add_custom_command(TARGET traintastic-server-test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${PROJECT_SOURCE_DIR}/thirdparty/lua5.3/bin/win64/lua53.dll" .)
elseif(APPLE)
  set(LUA_INCLUDE_DIR "/usr/local/opt/lua@5.3/include/lua")
  set(LUA_LIBRARIES "/usr/local/opt/lua@5.3/lib/liblua5.3.dylib")
else()
  find_package(Lua 5.3 REQUIRED)
endif()
target_include_directories(traintastic-server PRIVATE ${LUA_INCLUDE_DIR})
target_link_libraries(traintastic-server PRIVATE ${LUA_LIBRARIES})
target_include_directories(traintastic-server-test PRIVATE ${LUA_INCLUDE_DIR})
target_link_libraries(traintastic-server-test PRIVATE ${LUA_LIBRARIES})

### LIBRARIES END ###

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
set(CPACK_DEBIAN_PACKAGE_DEPENDS "traintastic-data (>= ${CPACK_PACKAGE_VERSION})")

include(CPack)
