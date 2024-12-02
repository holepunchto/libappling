cmake_minimum_required(VERSION 3.25)

# Force position independent code for all compilation units to allow linking
# the static libuv archive to the dynamic platform entry point.
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(cmake-bare REQUIRED PATHS node_modules/cmake-bare)
find_package(cmake-bare-bundle REQUIRED PATHS node_modules/cmake-bare-bundle)
find_package(cmake-fetch REQUIRED PATHS node_modules/cmake-fetch)
find_package(cmake-npm REQUIRED PATHS node_modules/cmake-npm)

project(appling C)

if(APPLE)
  enable_language(OBJC)
endif()

install_node_modules()

fetch_package("github:holepunchto/bare")
fetch_package("github:holepunchto/libpath")
fetch_package("github:holepunchto/libfs")
fetch_package("github:holepunchto/liblog")
fetch_package("github:holepunchto/libcompact")

add_library(appling OBJECT)

set_target_properties(
  appling
  PROPERTIES
  C_STANDARD 99
  POSITION_INDEPENDENT_CODE ON
)

if(APPLE)
  set_target_properties(
    appling
    PROPERTIES
    OBJC_STANDARD 99
  )
endif()

target_sources(
  appling
  INTERFACE
    include/appling.h
    include/appling/arch.h
    include/appling/constants.h
    include/appling/darwin.h
    include/appling/linux.h
    include/appling/os.h
    include/appling/win32.h
  PRIVATE
    src/launch.c
    src/lock.c
    src/unlock.c
    src/parse.c
    src/paths.c
    src/resolve.c
)

if(APPLE)
  target_sources(
    appling
    PRIVATE
      src/darwin/open.m
  )
endif()

if(WIN32)
  target_sources(
    appling
    PRIVATE
      src/win32/open.c
  )
endif()

if(LINUX)
  target_sources(
    appling
    PRIVATE
      src/linux/open.c
  )
endif()

target_include_directories(
  appling
  INTERFACE
    include
  PUBLIC
    $<TARGET_PROPERTY:uv,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:js,INTERFACE_INCLUDE_DIRECTORIES>
)

target_link_libraries(
  appling
  PUBLIC
    compact
    fs
    log
    path
)

add_bare_bundle(
  appling_bootstrap_bundle
  ENTRY src/bootstrap.js
  OUT src/bootstrap.bundle.h
  BUILTINS src/builtins.json
)

add_library(appling_bootstrap OBJECT)

set_target_properties(
  appling_bootstrap
  PROPERTIES
  C_STANDARD 99
  POSITION_INDEPENDENT_CODE ON
)

target_sources(
  appling_bootstrap
  PRIVATE
    src/bootstrap.c
    src/bootstrap.bundle.h
)

target_include_directories(
  appling_bootstrap
  PRIVATE
    $<TARGET_PROPERTY:bare,INTERFACE_INCLUDE_DIRECTORIES>
)

target_link_libraries(
  appling_bootstrap
  PUBLIC
    appling
    path
)

link_bare_modules(appling_bootstrap)

add_library(appling_static STATIC)

set_target_properties(
  appling_static
  PROPERTIES
  OUTPUT_NAME appling
  PREFIX lib
)

target_link_libraries(
  appling_static
  PUBLIC
    appling
    appling_bootstrap
  PRIVATE
    $<LINK_LIBRARY:WHOLE_ARCHIVE,bare_static>
    fs_static
    path_static
    log_static
    compact_static
)

add_library(appling_launch OBJECT)

set_target_properties(
  appling_launch
  PROPERTIES
  C_STANDARD 99
  POSITION_INDEPENDENT_CODE ON
)

target_sources(
  appling_launch
  PRIVATE
    src/entry.c
)

target_link_libraries(
  appling_launch
  PUBLIC
    appling
    log
    path
)

add_library(appling_launch_shared SHARED)

set_target_properties(
  appling_launch_shared
  PROPERTIES
  OUTPUT_NAME launch
  PREFIX ""
  WINDOWS_EXPORT_ALL_SYMBOLS ON
)

target_link_libraries(
  appling_launch_shared
  PUBLIC
    appling_launch
  PRIVATE
    log_static
    path_static
)

install(TARGETS appling_static)

install(FILES include/appling.h DESTINATION include)

install(DIRECTORY include/appling DESTINATION include)

if(PROJECT_IS_TOP_LEVEL)
  enable_testing()

  add_subdirectory(test)
endif()
