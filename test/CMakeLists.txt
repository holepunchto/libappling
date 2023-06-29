list(APPEND tests
  bootstrap-dangling-tmp
  bootstrap-existing-platform
  bootstrap-no-platform
  extract
  launch
  launch-data
  launch-known-app
  launch-known-link
  launch-unknown-app
  launch-unknown-link
  lock
  lock-non-existing
  parse-hex
  parse-invalid
  parse-z32
  resolve-both
  resolve-current
  resolve-next
)

add_subdirectory(fixtures)

foreach(test IN LISTS tests)
  add_executable(${test} ${test}.c)

  target_link_libraries(
    ${test}
    PRIVATE
      appling_static
  )

  target_include_directories(
    ${test}
    PRIVATE
      $<TARGET_PROPERTY:appling,INTERFACE_INCLUDE_DIRECTORIES>
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
