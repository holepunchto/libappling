target_link_options(
  example
  PRIVATE
    /subsystem:windows
    /entry:mainCRTStartup
)

install(
  TARGETS example
  RUNTIME DESTINATION Example
)

install(
  DIRECTORY Example/resources
  DESTINATION Example
)
