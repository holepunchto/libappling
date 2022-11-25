list(APPEND tests
  bootstrap-newer-platform
  bootstrap-no-platform
  bootstrap-older-platform
  extract
  lock
  parse
  resolve
)

foreach(test IN LISTS tests)
  add_executable(${test} ${test}.c)

  target_link_libraries(${test} holepunch_static)

  target_include_directories(
    ${test}
    PRIVATE
      $<TARGET_PROPERTY:holepunch,INCLUDE_DIRECTORIES>
  )

  add_test(
    NAME ${test}
    COMMAND ${test}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  )
endforeach()
