cmake_minimum_required(VERSION 3.22)

project(example_darwin C)

set(APP ${PROJECT_SOURCE_DIR}/Example.app)

add_executable(
  example_darwin
  ../main.c
  ${APP}/Contents/Info.plist
  ${APP}/Contents/Resources/app/checkout
  ${APP}/Contents/Resources/app/platform.mp3
)

target_include_directories(
  example_darwin
  PRIVATE
    $<TARGET_PROPERTY:appling,INCLUDE_DIRECTORIES>
)

target_compile_definitions(
  example_darwin
  PRIVATE
    EXAMPLE_APPLING_PLATFORM_DIR="${PROJECT_SOURCE_DIR}/../../holepunch"
)

set_target_properties(
  example_darwin
  PROPERTIES
  OUTPUT_NAME Example
  MACOSX_BUNDLE YES
  MACOSX_BUNDLE_INFO_PLIST "${APP}/Contents/Info.plist"
  XCODE_ATTRIBUTE_PRODUCT_NAME "Example"
  XCODE_ATTRIBUTE_BUNDLE_IDENTIFIER "com.example.Example"
)

set_source_files_properties(
  ${APP}/Contents/Resources/app/checkout
  PROPERTIES
  MACOSX_PACKAGE_LOCATION "Resources/app"
)

set_source_files_properties(
  ${APP}/Contents/Resources/app/platform.mp3
  PROPERTIES
  MACOSX_PACKAGE_LOCATION "Resources/app"
)

target_link_libraries(
  example_darwin
  PRIVATE
    appling_static
)
