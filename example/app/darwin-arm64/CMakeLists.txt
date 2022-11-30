cmake_minimum_required(VERSION 3.22)

project(example_darwin_arm64 C)

add_executable(example_darwin_arm64 ../main.c)

target_include_directories(
  example_darwin_arm64
  PRIVATE
    $<TARGET_PROPERTY:appling,INCLUDE_DIRECTORIES>
)

target_compile_definitions(
  example_darwin_arm64
  PRIVATE
    EXAMPLE_APPLING_PLATFORM_DIR="${PROJECT_SOURCE_DIR}/../../holepunch"
)

set_target_properties(
  example_darwin_arm64
  PROPERTIES
  OUTPUT_NAME Example
)

target_link_libraries(
  example_darwin_arm64
  PRIVATE
    appling_static
)

install(
  TARGETS example_darwin_arm64
  RUNTIME DESTINATION Example.app/Contents/MacOS
)

install(
  DIRECTORY Example.app/Contents
  DESTINATION Example.app
)
