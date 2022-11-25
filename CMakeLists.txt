cmake_minimum_required(VERSION 3.22)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(WIN32)
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

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

add_library(appling OBJECT)

set_target_properties(
  appling
  PROPERTIES
  C_STANDARD 99
  POSITION_INDEPENDENT_CODE 1
)

target_sources(
  appling
  INTERFACE
    include/appling.h
    include/appling/arch.h
    include/appling/constants.h
    include/appling/linux.h
    include/appling/mac.h
    include/appling/os.h
    include/appling/win.h
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
    $<TARGET_PROPERTY:uv,INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:fs,INCLUDE_DIRECTORIES>
  PRIVATE
    $<TARGET_PROPERTY:archive,INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:path,INCLUDE_DIRECTORIES>
)

add_library(appling_static STATIC $<TARGET_OBJECTS:appling>)

set_target_properties(
  appling_static
  PROPERTIES
  OUTPUT_NAME appling
)

target_link_libraries(
  appling_static
  PUBLIC
    uv
  PRIVATE
    archive_static
    fs_static
    path_static
)

install(TARGETS appling_static)

install(FILES include/appling.h DESTINATION include)
install(DIRECTORY include/appling DESTINATION include)

if(PROJECT_IS_TOP_LEVEL)
  enable_testing()
  add_subdirectory(test)

  if(APPLE)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
      add_subdirectory(example/app/darwin-arm64 EXCLUDE_FROM_ALL)
    else()
      add_subdirectory(example/app/darwin-x64 EXCLUDE_FROM_ALL)
    endif()
  endif()
endif()
