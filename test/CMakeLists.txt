list(APPEND tests
  bootstrap-no-platform
  launch
  launch-data
  lock
  lock-non-existing
  parse-hex
  parse-invalid
  parse-named
  parse-z32
  paths
  resolve-both
  resolve-both-minimum-length
  resolve-both-minimum-length-mismatch
  resolve-current
  resolve-current-minimum-length
  resolve-current-minimum-length-mismatch
  resolve-next
)

if(WIN32)
  list(APPEND skipped_tests
    # Blocked by Windows Defender
    bootstrap-no-platform
  )
endif()

add_subdirectory(fixtures)

foreach(test IN LISTS tests)
  add_executable(${test} ${test}.c fixtures/app.h)

  target_link_libraries(
    ${test}
    PRIVATE
      appling_static
      compact_static
  )

  add_test(
    NAME ${test}
    COMMAND ${test}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  )

  set_tests_properties(
    ${test}
    PROPERTIES
    TIMEOUT 600
  )

  if(${test} IN_LIST skipped_tests)
    set_tests_properties(
      ${test}
      PROPERTIES
      DISABLED True
    )
  endif()
endforeach()
