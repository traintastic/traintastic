cmake_minimum_required(VERSION 3.9)
include(../shared/traintastic.cmake)
project(traintastic-simulator VERSION ${TRAINTASTIC_VERSION} DESCRIPTION "Traintastic simulator")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

configure_file(../shared/src/traintastic/version.hpp.in version.hpp)

if(MSVC)
  add_compile_options(/W4 /WX)
else()
  add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Werror)
endif()

add_executable(traintastic-simulator
  gfx/common.qrc
  src/main.cpp
  src/mainwindow.cpp
  src/simulator.cpp
  src/simulatorview.cpp
  ../shared/src/traintastic/utils/standardpaths.cpp
)

add_definitions(-DQT_DEPRECATED_WARNINGS)

target_include_directories(traintastic-simulator PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}
  ../shared/src
)

target_include_directories(traintastic-simulator SYSTEM PRIVATE
  ../shared/thirdparty
)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Xml OpenGLWidgets Network OpenGL)
message(STATUS "Found Qt ${Qt6Core_VERSION}")
target_link_libraries(traintastic-simulator PRIVATE Qt6::Core Qt6::Widgets Qt6::Xml Qt6::OpenGLWidgets Qt6::Network Qt6::OpenGL)

if(WIN32)
  target_link_libraries(traintastic-simulator PRIVATE OpenGL32)
  target_sources(traintastic-simulator PRIVATE ../shared/gfx/appicon.rc src/version.rc)
  set_property(TARGET traintastic-simulator PROPERTY WIN32_EXECUTABLE true)
endif()

### INSTALL ###

install(TARGETS traintastic-simulator
    COMPONENT traintastic-simulator
    RUNTIME DESTINATION "/opt/traintastic/bin/"
)

if(LINUX)
  install(FILES ${CMAKE_SOURCE_DIR}/debian/traintastic-simulator.desktop DESTINATION share/applications)
  install(FILES ${CMAKE_SOURCE_DIR}/debian/traintastic-simulator_256.png DESTINATION "/opt/traintastic/icons/")
endif()

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
