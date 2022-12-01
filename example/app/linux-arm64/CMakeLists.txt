install(
  TARGETS example
  RUNTIME DESTINATION Example.AppDir
)

install(
  DIRECTORY Example.AppDir/resources
  DESTINATION Example.AppDir
)
