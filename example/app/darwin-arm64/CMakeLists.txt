set_target_properties(
  example
  PROPERTIES
  OUTPUT_NAME Example
)

install(
  TARGETS example
  RUNTIME DESTINATION Example.app/Contents/MacOS
)

install(
  DIRECTORY Example.app/Contents
  DESTINATION Example.app
)
