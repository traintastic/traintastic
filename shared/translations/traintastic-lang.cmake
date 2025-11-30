find_package(Python3)

file(GLOB TRANSLATION_SOURCE_FILES "${CMAKE_CURRENT_LIST_DIR}/*.json")

foreach(TRANSLATION_FILE ${TRANSLATION_SOURCE_FILES})
  get_filename_component(TRANSLATION_FILE_NAME ${TRANSLATION_FILE} NAME_WE)
  set(LANG_FILE ${CMAKE_CURRENT_LIST_DIR}/${TRANSLATION_FILE_NAME}.lang)
  add_custom_command(
    OUTPUT ${LANG_FILE}
    COMMAND Python3::Interpreter json2lang.py ${TRANSLATION_FILE} ${LANG_FILE}
    DEPENDS ${TRANSLATION_FILE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    
  )
  list(APPEND TRANSLATION_FILES ${LANG_FILE})
endforeach()

add_custom_target(
  traintastic-lang ALL
  DEPENDS ${TRANSLATION_FILES}
  COMMENT "Updating language files"
)
