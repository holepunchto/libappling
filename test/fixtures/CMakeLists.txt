add_executable(runtime runtime.c)

file(CREATE_LINK
  by-dkey/aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa/0
  ${CMAKE_CURRENT_LIST_DIR}/platform/current
  SYMBOLIC
)
