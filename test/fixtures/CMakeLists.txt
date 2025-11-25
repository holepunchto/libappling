bare_target(target)

install(
  TARGETS appling_launch_shared
  DESTINATION ${CMAKE_CURRENT_LIST_DIR}/platform/by-dkey/aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa/0/by-arch/${target}/lib
  COMPONENT fixtures
)

add_executable(appling_platform_runtime runtime.c)

set_target_properties(
  appling_platform_runtime
  PROPERTIES
  OUTPUT_NAME pear-runtime
)

install(
  TARGETS appling_platform_runtime
  DESTINATION ${CMAKE_CURRENT_LIST_DIR}/platform/by-dkey/aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa/0/by-arch/${target}/bin
  COMPONENT fixtures
)

file(
  CREATE_LINK
  ${CMAKE_CURRENT_LIST_DIR}/platform/by-dkey/aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa/0
  ${CMAKE_CURRENT_LIST_DIR}/platform/current
  SYMBOLIC
)

file(
  CREATE_LINK
  ${CMAKE_CURRENT_LIST_DIR}/platform/by-dkey/aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa/0
  ${CMAKE_CURRENT_LIST_DIR}/resolve/current/current
  SYMBOLIC
)

file(
  CREATE_LINK
  ${CMAKE_CURRENT_LIST_DIR}/platform/by-dkey/aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa/0
  ${CMAKE_CURRENT_LIST_DIR}/resolve/both/current
  SYMBOLIC
)

file(
  CREATE_LINK
  ${CMAKE_CURRENT_LIST_DIR}/platform/by-dkey/aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa/1
  ${CMAKE_CURRENT_LIST_DIR}/resolve/both/next
  SYMBOLIC
)

file(
  CREATE_LINK
  ${CMAKE_CURRENT_LIST_DIR}/platform/by-dkey/aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa/1
  ${CMAKE_CURRENT_LIST_DIR}/resolve/next/next
  SYMBOLIC
)

execute_process(
  COMMAND node ${CMAKE_CURRENT_LIST_DIR}/applings.js
)

execute_process(
  COMMAND node ${CMAKE_CURRENT_LIST_DIR}/checkouts.js
)
