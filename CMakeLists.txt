cmake_minimum_required(VERSION 3.25)

project(appling C)

if(APPLE)
  enable_language(OBJC)
endif()

include(bare)

# Force position independent code for all compilation units to allow linking
# the static libuv archive to the dynamic platform entry point.
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

install_node_modules(LOCKFILE)

if(NOT TARGET uv)
  add_subdirectory(vendor/libuv EXCLUDE_FROM_ALL)
endif()

if(NOT TARGET path)
  add_subdirectory(vendor/libpath EXCLUDE_FROM_ALL)
endif()

if(NOT TARGET fs)
  add_subdirectory(vendor/libfs EXCLUDE_FROM_ALL)
endif()

if(NOT TARGET log)
  add_subdirectory(vendor/liblog EXCLUDE_FROM_ALL)
endif()

if(NOT TARGET hex)
  add_subdirectory(vendor/libhex EXCLUDE_FROM_ALL)
endif()

if(NOT TARGET compact)
  add_subdirectory(vendor/libcompact EXCLUDE_FROM_ALL)
endif()

if(NOT TARGET bare)
  add_subdirectory(vendor/bare EXCLUDE_FROM_ALL)
endif()

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
  PUBLIC
    include
    $<TARGET_PROPERTY:uv,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:fs,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:bare,INTERFACE_INCLUDE_DIRECTORIES>
  PRIVATE
    $<TARGET_PROPERTY:hex,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:path,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:log,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:compact,INTERFACE_INCLUDE_DIRECTORIES>
)

add_bare_bundle(
  ENTRY src/bootstrap.js
  OUT src/bootstrap.bundle.h
  TARGET c
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
  PUBLIC
    include
    $<TARGET_PROPERTY:uv,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:bare,INTERFACE_INCLUDE_DIRECTORIES>
  PRIVATE
    $<TARGET_PROPERTY:hex,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:path,INTERFACE_INCLUDE_DIRECTORIES>
)

target_link_libraries(
  appling_bootstrap
  PUBLIC
    appling
)

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
    hex_static
    compact_static
)

link_bare_modules(appling_static)

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

target_include_directories(
  appling_launch
  PRIVATE
    $<TARGET_PROPERTY:path,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:log,INTERFACE_INCLUDE_DIRECTORIES>
)

target_link_libraries(
  appling_launch
  PUBLIC
    appling
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
  PRIVATE
    appling_launch
    path_static
    log_static
)

install(TARGETS appling_static)

install(FILES include/appling.h DESTINATION include)

install(DIRECTORY include/appling DESTINATION include)

if(PROJECT_IS_TOP_LEVEL)
  enable_testing()

  add_subdirectory(test)
endif()
