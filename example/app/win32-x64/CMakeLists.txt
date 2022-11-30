cmake_minimum_required(VERSION 3.22)

project(example_win32_x64 C)

add_executable(example_win32_x64 ../main.c)

target_include_directories(
  example_win32_x64
  PRIVATE
    $<TARGET_PROPERTY:appling,INCLUDE_DIRECTORIES>
)

target_compile_definitions(
  example_win32_x64
  PRIVATE
    EXAMPLE_APPLING_PLATFORM_DIR="${PROJECT_SOURCE_DIR}/../../holepunch"
)

set_target_properties(
  example_win32_x64
  PROPERTIES
  OUTPUT_NAME Example
)

target_link_libraries(
  example_win32_x64
  PRIVATE
    appling_static
)

install(
  TARGETS example_win32_x64
  RUNTIME DESTINATION Example
)

install(
  DIRECTORY resources
  DESTINATION Example
)
