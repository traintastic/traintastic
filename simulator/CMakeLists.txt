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
