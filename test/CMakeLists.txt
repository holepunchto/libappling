list(APPEND tests
  bootstrap-newer-platform
  bootstrap-no-platform
  bootstrap-older-platform
  extract
  launch
  launch-link
  launch-link-data
  launch-link-different-key
  lock
  lock-non-existing
  parse-hex
  parse-invalid
  parse-z32
  resolve
)

add_subdirectory(fixtures)

foreach(test IN LISTS tests)
  add_executable(${test} ${test}.c)

  target_link_libraries(
    ${test}
    PRIVATE
      appling_static
  )

  add_test(
    NAME ${test}
    COMMAND ${test}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  )

  set_tests_properties(
    ${test}
    PROPERTIES
    TIMEOUT 30000
  )
endforeach()
