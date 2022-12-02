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
  parse
  parse-invalid
  resolve
)

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
      $<TARGET_PROPERTY:appling,INCLUDE_DIRECTORIES>
  )

  add_test(
    NAME ${test}
    COMMAND ${test}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  )
endforeach()

add_subdirectory(fixtures)
