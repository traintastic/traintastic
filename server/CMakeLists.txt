cmake_minimum_required(VERSION 3.18)
include(../shared/traintastic.cmake)
project(traintastic-server VERSION ${TRAINTASTIC_VERSION} DESCRIPTION "Traintastic server")
include(GNUInstallDirs)
include(CTest)
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
  set(CMAKE_CXX_CLANG_TIDY clang-tidy)
endif()

add_executable(traintastic-server src/main.cpp src/options.hpp)
add_dependencies(traintastic-server traintastic-lang)
set_target_properties(traintastic-server PROPERTIES CXX_STANDARD 20)
target_include_directories(traintastic-server PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}
  ../shared/src)
target_include_directories(traintastic-server SYSTEM PRIVATE
  ../shared/thirdparty
  thirdparty)

if(BUILD_TESTING)
  add_subdirectory(thirdparty/catch2)
  set_target_properties(Catch2 PROPERTIES
    CXX_STANDARD 20
    CXX_CLANG_TIDY ""
  )
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(Catch2 PRIVATE -Wno-restrict) # workaround GCC bug
  endif()
  add_executable(traintastic-server-test test/main.cpp)
  add_dependencies(traintastic-server-test traintastic-lang)
  target_compile_definitions(traintastic-server-test PRIVATE -DTRAINTASTIC_TEST)
  set_target_properties(traintastic-server-test PROPERTIES
    CXX_STANDARD 20
    CXX_CLANG_TIDY ""
  )
  target_include_directories(traintastic-server-test PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    ../shared/src)
  target_include_directories(traintastic-server-test SYSTEM PRIVATE
    ../shared/thirdparty
    thirdparty)
  target_link_libraries(traintastic-server-test PRIVATE Catch2::Catch2WithMain)
endif()

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
  "src/board/tile/hidden/*.cpp"
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
  "src/hardware/protocol/dccex/*.hpp"
  "src/hardware/protocol/dccex/*.cpp"
  "src/hardware/protocol/dccex/iohandler/*.hpp"
  "src/hardware/protocol/dccex/iohandler/*.cpp"
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
  "src/throttle/*.hpp"
  "src/throttle/*.cpp"
  "src/throttle/list/*.hpp"
  "src/throttle/list/*.cpp"
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

### VCPKG
if(DEFINED ENV{VCPKG_ROOT})
  message(STATUS "Using VCPKG (VCPKG_ROOT=$ENV{VCPKG_ROOT})")
  if(WIN32)
    set(VCPKG_TARGET_TRIPLET "x64-windows-static-md")
  endif()
  include($ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)
endif()

### RESOURCES ###

include(cmake/add-resource.cmake)

add_resource(resource-www
  FILES
    www/css/normalize.css
    www/css/throttle.css
    www/js/throttle.js
    www/throttle.html
)

add_resource(resource-shared
  BASE_DIR ../
  FILES
    shared/gfx/appicon.ico
)

add_dependencies(traintastic-server resource-www resource-shared)
if(BUILD_TESTING)
  add_dependencies(traintastic-server-test resource-www resource-shared)
endif()

### OPTIONS ###

if(NO_LOCALHOST_ONLY_SETTING)
  message(STATUS "Setting localhost only removed")
  add_definitions(-DNO_LOCALHOST_ONLY_SETTING)
endif()

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
  pkg_check_modules(LIBSYSTEMD IMPORTED_TARGET libsystemd)
  if(${LIBSYSTEMD_FOUND})
    add_definitions(-DHAS_LIBSYSTEMD)
    list(APPEND SOURCES "src/os/linux/serialportlistimplsystemd.hpp" "src/os/linux/serialportlistimplsystemd.cpp")
    target_link_libraries(traintastic-server PRIVATE PkgConfig::LIBSYSTEMD)
    if(BUILD_TESTING)
      target_link_libraries(traintastic-server-test PRIVATE PkgConfig::LIBSYSTEMD)
    endif()
  else()
    # Use inotify for monitoring serial ports:
    list(APPEND SOURCES "src/os/linux/serialportlistimplinotify.hpp" "src/os/linux/serialportlistimplinotify.cpp")
  endif()
else()
  # socket CAN is only available on linux:
  file(GLOB SOCKET_CAN_SOURCES "src/hardware/protocol/marklincan/iohandler/socketcaniohandler.*")
  list(REMOVE_ITEM SOURCES ${SOCKET_CAN_SOURCES})
endif()

if(APPLE)
  add_definitions(-D_LIBCPP_ENABLE_CXX17_REMOVED_UNARY_BINARY_FUNCTION) # remove once boost is updated
endif()

if(WIN32)
  add_definitions(-D_WIN32_WINNT=0x0601)
  file(GLOB SOURCES_WINDOWS "src/os/windows/*.hpp" "src/os/windows/*.cpp")
  list(APPEND SOURCES ${SOURCES_WINDOWS} src/version.rc ../shared/gfx/appicon.rc)
  target_link_libraries(traintastic-server PRIVATE bcrypt setupapi)
  if(BUILD_TESTING)
    target_link_libraries(traintastic-server-test PRIVATE bcrypt setupapi)
  endif()
endif()

### COMPILER ###

if(MSVC)

else()
  ### Enable ASAN and UBSAN
  #set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined")

  ### Use the LLVM lld linker (faster than standard linker)
  #set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld")

  target_link_libraries(traintastic-server PRIVATE pthread)
  if(BUILD_TESTING)
    target_link_libraries(traintastic-server-test PRIVATE pthread)
  endif()

  if(NOT APPLE)
    target_link_libraries(traintastic-server PRIVATE stdc++fs)
    if(BUILD_TESTING)
      target_link_libraries(traintastic-server-test PRIVATE stdc++fs)
    endif()
  endif()
endif()

### LIBRARIES ###

if(WIN32 AND NOT MSVC)
  # On MinGW ws2_32.dll needs explicit linking
  # Also mswsock.dll is needed for AcceptEx() used by Boost.Asio
  target_link_libraries(traintastic-server PRIVATE ws2_32 mswsock)
  if(BUILD_TESTING)
    target_link_libraries(traintastic-server-test PRIVATE ws2_32 mswsock)
  endif()
endif()

# boost
find_package(Boost 1.81 REQUIRED COMPONENTS program_options)
target_include_directories(traintastic-server SYSTEM PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(traintastic-server PRIVATE ${Boost_LIBRARIES})
if(BUILD_TESTING)
  target_include_directories(traintastic-server-test SYSTEM PRIVATE ${Boost_INCLUDE_DIRS})
  target_link_libraries(traintastic-server-test PRIVATE ${Boost_LIBRARIES})
endif()

# zlib
find_package(ZLIB REQUIRED)
target_link_libraries(traintastic-server PRIVATE ZLIB::ZLIB)
if(BUILD_TESTING)
  target_link_libraries(traintastic-server-test PRIVATE ZLIB::ZLIB)
endif()

# libarchive
find_package(LibArchive REQUIRED)
target_link_libraries(traintastic-server PRIVATE LibArchive::LibArchive)
if(BUILD_TESTING)
  target_link_libraries(traintastic-server-test PRIVATE LibArchive::LibArchive)
endif()

# lua
find_package(Lua REQUIRED)
target_include_directories(traintastic-server PRIVATE ${LUA_INCLUDE_DIR})
target_link_libraries(traintastic-server PRIVATE ${LUA_LIBRARIES})
if(BUILD_TESTING)
  target_include_directories(traintastic-server-test PRIVATE ${LUA_INCLUDE_DIR})
  target_link_libraries(traintastic-server-test PRIVATE ${LUA_LIBRARIES})
endif()

### LIBRARIES END ###

target_sources(traintastic-server PRIVATE ${SOURCES})
if(BUILD_TESTING)
  target_sources(traintastic-server-test PRIVATE ${TEST_SOURCES} ${SOURCES})
endif()

### CODE COVERAGE ###

target_code_coverage(traintastic-server-test AUTO EXCLUDE "${PROJECT_SOURCE_DIR}/test/*" "${PROJECT_SOURCE_DIR}/thirdparty/*")

### INSTALL ###

install(TARGETS traintastic-server
    COMPONENT traintastic-server
    RUNTIME DESTINATION "/opt/traintastic/bin/"
)

if(INSTALL_TRANSLATIONS)
  install(FILES ${TRANSLATION_FILES} DESTINATION "/opt/traintastic/translations")
endif()

if(INSTALL_SYSTEMD_SERVICE)
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/debian/traintastic-server.service DESTINATION "/lib/systemd/system")
endif()

### TEST ###

if(BUILD_TESTING)
  include(Catch)
  catch_discover_tests(traintastic-server-test DISCOVERY_MODE PRE_TEST)
endif()

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
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/debian/postinst")

include(CPack)
