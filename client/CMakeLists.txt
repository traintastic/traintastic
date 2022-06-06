cmake_minimum_required(VERSION 3.9)
include(../shared/traintastic.cmake)
project(traintastic-client VERSION ${TRAINTASTIC_VERSION} DESCRIPTION "Traintastic client")
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

configure_file(../shared/src/traintastic/version.hpp.in version.hpp)

if(MSVC)
  add_compile_options(/Wall)
else()
  add_compile_options(-Wall -Wextra -Wpedantic -Wshadow)
endif()

add_executable(traintastic-client
  gfx/common.qrc
  gfx/dark/dark.qrc
  gfx/light/light.qrc)

add_definitions(-DQT_DEPRECATED_WARNINGS)

target_include_directories(traintastic-client PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}
  ../shared/src)

target_include_directories(traintastic-client SYSTEM PRIVATE
  ../shared/thirdparty
  thirdparty)

file(GLOB SOURCES
  "src/*.hpp"
  "src/*.cpp"
  "src/board/*.hpp"
  "src/board/*.cpp"
  "src/dialog/*.hpp"
  "src/dialog/*.cpp"
  "src/misc/*.hpp"
  "src/misc/*.cpp"
  "src/network/*.hpp"
  "src/network/*.cpp"
  "src/settings/*.hpp"
  "src/settings/*.cpp"
  "src/style/*.hpp"
  "src/style/*.cpp"
  "src/subwindow/*.hpp"
  "src/subwindow/*.cpp"
  "src/theme/*.hpp"
  "src/theme/*.cpp"
  "src/utils/*.hpp"
  "src/utils/*.cpp"
  "src/widget/*.hpp"
  "src/widget/*.cpp"
  "src/widget/object/*.hpp"
  "src/widget/object/*.cpp"
  "src/widget/objectlist/*.hpp"
  "src/widget/objectlist/*.cpp"
  "src/widget/throttle/*.hpp"
  "src/widget/throttle/*.cpp"
  "../shared/src/traintastic/locale/locale.cpp"
  "../shared/src/traintastic/utils/standardpaths.cpp"
  "thirdparty/QtWaitingSpinner/*.hpp"
  "thirdparty/QtWaitingSpinner/*.cpp")

find_package(Qt5 COMPONENTS Widgets Network Svg REQUIRED)

target_sources(traintastic-client PRIVATE ${SOURCES})

target_link_libraries(traintastic-client PRIVATE Qt5::Widgets Qt5::Network Qt5::Svg)

### PLATFORM ###

if(WIN32)
  target_sources(traintastic-client PRIVATE ../shared/gfx/appicon.rc src/version.rc)
  set_property(TARGET traintastic-client PROPERTY WIN32_EXECUTABLE true)
endif()

### COMPILER ###

if(MSVC)

else()

  if(NOT APPLE)
    target_link_libraries(traintastic-client PRIVATE stdc++fs)
  endif()
endif()

### INSTALL ###

install(TARGETS traintastic-client
    COMPONENT traintastic-client
    RUNTIME DESTINATION "/opt/traintastic/bin/"
)

### BUILD TYPE ###

if (CMAKE_BUILD_TYPE EQUAL "Release")
  add_definitions(-DQT_NO_DEBUG_OUTPUT)
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
