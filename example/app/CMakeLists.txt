cmake_minimum_required(VERSION 3.22)

project(example C)

add_executable(example main.c)

target_include_directories(
  example
  PRIVATE
    $<TARGET_PROPERTY:appling,INCLUDE_DIRECTORIES>
)

target_compile_definitions(
  example
  PRIVATE
    PLATFORM_DIR="${PROJECT_SOURCE_DIR}/../holepunch"
)

set_target_properties(
  example
  PROPERTIES
  OUTPUT_NAME Example
)

target_link_libraries(
  example
  PRIVATE
    appling_static
)

if(APPLE)
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
    add_subdirectory(darwin-arm64)
  else()
    add_subdirectory(darwin-x64)
  endif()
endif()

if(WIN32)
  add_subdirectory(win32-x64)
endif()
