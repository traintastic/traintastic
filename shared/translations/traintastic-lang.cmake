find_package(Python3)

file(GLOB TRANSLATION_FILES "${CMAKE_CURRENT_LIST_DIR}/*.json")
list(TRANSFORM TRANSLATION_FILES REPLACE "[.]json$" ".lang")

add_custom_target(
  traintastic-lang ALL
  Python3::Interpreter json2lang.py
  COMMENT "Updating language files"
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)
