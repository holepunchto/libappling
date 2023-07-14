add_executable(runtime runtime.c)

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
