cmake_minimum_required(VERSION 3.9)
include(./traintastic.cmake)
project(traintastic-data VERSION ${TRAINTASTIC_VERSION} DESCRIPTION "Traintastic data")

include(translations/traintastic-lang.cmake)

### INSTALL ###

install(FILES ${TRANSLATION_FILES} DESTINATION "/opt/traintastic/translations")

file(GLOB LNCV_XML_FILES "data/lncv/xml/*.xml")
install(FILES ${LNCV_XML_FILES} "data/lncv/xml/lncvmodule.xsd" DESTINATION "/opt/traintastic/lncv")

install(DIRECTORY "../manual/output/" DESTINATION "/opt/traintastic/manual")

### DEB package ###

if(NOT DEBIAN_PACKAGE_VERSION_EXTRA)
  set(DEBIAN_PACKAGE_VERSION_EXTRA "${PROJECT_VERSION_EXTRA}")
endif()

set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}${DEBIAN_PACKAGE_VERSION_EXTRA}")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "all")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_DEBIAN_PACKAGE_SECTION "misc")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Reinder Feenstra")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://traintastic.org")

include(CPack)
