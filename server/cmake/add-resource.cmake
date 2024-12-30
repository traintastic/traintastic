#
# This file is part of the traintastic source code.
# See <https://github.com/traintastic/traintastic>.
#
# Copyright (C) 2024 Reinder Feenstra
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#

function(add_resource TARGET_NAME)
  cmake_parse_arguments(PARSE_ARG "" "BASE_DIR" "FILES" ${ARGN})
  if(PARSE_ARG_BASE_DIR)
    set(PARSE_ARG_BASE_DIR "${CMAKE_SOURCE_DIR}/${PARSE_ARG_BASE_DIR}")
  else()
    set(PARSE_ARG_BASE_DIR "${CMAKE_SOURCE_DIR}")
  endif()
  foreach(INPUT_FILE ${PARSE_ARG_FILES})
    set(OUTPUT_FILE ${CMAKE_BINARY_DIR}/resource/${INPUT_FILE}.hpp)
    add_custom_command(
        OUTPUT ${OUTPUT_FILE}
        COMMAND Python3::Interpreter ${CMAKE_SOURCE_DIR}/cmake/generateresourceheader.py ${PARSE_ARG_BASE_DIR} ${INPUT_FILE} ${OUTPUT_FILE}
        DEPENDS ${CMAKE_SOURCE_DIR}/cmake/generateresourceheader.py ${PARSE_ARG_BASE_DIR}/${INPUT_FILE}
        COMMENT "Generating resource header resource/${INPUT_FILE}.hpp"
    )
    list(APPEND OUTPUT_HEADERS ${OUTPUT_FILE})
  endforeach()
  add_custom_target(${TARGET_NAME} ALL DEPENDS ${OUTPUT_HEADERS})
endfunction()
