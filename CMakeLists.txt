cmake_minimum_required(VERSION 3.22)

project(appling C)

option(ENABLE_ACL OFF)
option(ENABLE_BZip2 OFF)
option(ENABLE_CAT OFF)
option(ENABLE_CNG OFF)
option(ENABLE_CPIO OFF)
option(ENABLE_EXPAT OFF)
option(ENABLE_ICONV OFF)
option(ENABLE_LIBB2 OFF)
option(ENABLE_LibGCC OFF)
option(ENABLE_LIBXML2 OFF)
option(ENABLE_LZ4 OFF)
option(ENABLE_LZMA OFF)
option(ENABLE_LZO OFF)
option(ENABLE_OPENSSL OFF)
option(ENABLE_PCREPOSIX OFF)
option(ENABLE_TEST OFF)
option(ENABLE_XATTR OFF)
option(ENABLE_ZLIB OFF)
option(ENABLE_ZSTD OFF)

if(NOT TARGET uv)
  add_subdirectory(vendor/libuv EXCLUDE_FROM_ALL)
endif()

if(NOT TARGET archive)
  add_subdirectory(vendor/libarchive EXCLUDE_FROM_ALL)
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

if(NOT TARGET z32)
  add_subdirectory(vendor/libz32 EXCLUDE_FROM_ALL)
endif()

add_library(appling OBJECT)

set_target_properties(
  appling
  PROPERTIES
  C_STANDARD 99
  POSITION_INDEPENDENT_CODE ON
)

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
    src/bootstrap.c
    src/extract.c
    src/launch.c
    src/lock.c
    src/unlock.c
    src/parse.c
    src/resolve.c
)

target_include_directories(
  appling
  PUBLIC
    include
    $<TARGET_PROPERTY:uv,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:fs,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:hex,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:z32,INTERFACE_INCLUDE_DIRECTORIES>
  PRIVATE
    $<TARGET_PROPERTY:archive,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:path,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:log,INTERFACE_INCLUDE_DIRECTORIES>
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

target_include_directories(
  appling_launch
  PUBLIC
    include
    $<TARGET_PROPERTY:appling,INTERFACE_INCLUDE_DIRECTORIES>
  PRIVATE
    $<TARGET_PROPERTY:path,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:log,INTERFACE_INCLUDE_DIRECTORIES>
)

add_library(appling_static STATIC $<TARGET_OBJECTS:appling>)

set_target_properties(
  appling_static
  PROPERTIES
  OUTPUT_NAME appling
  PREFIX lib
)

target_link_libraries(
  appling_static
  PUBLIC
    uv_a
  PRIVATE
    archive_static
    fs_static
    path_static
    log_static
    hex_static
    z32_static
)

add_library(appling_launch_shared SHARED $<TARGET_OBJECTS:appling_launch>)

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
    uv
    path_shared
    log_shared
    hex_shared
)

install(TARGETS appling_static)

install(FILES include/appling.h DESTINATION include)

install(DIRECTORY include/appling DESTINATION include)

if(PROJECT_IS_TOP_LEVEL)
  enable_testing()
  add_subdirectory(test)
  add_subdirectory(example/app)
endif()
