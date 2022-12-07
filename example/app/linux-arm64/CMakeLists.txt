set_target_properties(
  example
  PROPERTIES
  OUTPUT_NAME example
)

install(
  TARGETS example
  RUNTIME DESTINATION Example.AppDir/usr/bin
)

install(
  DIRECTORY Example.AppDir/resources
  DESTINATION Example.AppDir
)
