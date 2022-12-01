install(
  TARGETS example
  RUNTIME DESTINATION Example.app/Contents/MacOS
)

install(
  DIRECTORY Example.app/Contents
  DESTINATION Example.app
)
