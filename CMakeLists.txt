cmake_minimum_required(VERSION 3.22)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(WIN32)
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

project(holepunch C)

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

add_library(holepunch OBJECT)

set_target_properties(
  holepunch
  PROPERTIES
  C_STANDARD 99
  POSITION_INDEPENDENT_CODE 1
)

target_sources(
  holepunch
  INTERFACE
    include/holepunch.h
    include/holepunch/arch.h
    include/holepunch/constants.h
    include/holepunch/linux.h
    include/holepunch/mac.h
    include/holepunch/os.h
    include/holepunch/win.h
  PRIVATE
    src/bootstrap.c
    src/extract.c
    src/resolve.c
)

target_include_directories(
  holepunch
  PUBLIC
    include
    $<TARGET_PROPERTY:uv,INCLUDE_DIRECTORIES>
  PRIVATE
    $<TARGET_PROPERTY:archive,INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:path,INCLUDE_DIRECTORIES>
)

add_library(holepunch_static STATIC $<TARGET_OBJECTS:holepunch>)

set_target_properties(
  holepunch_static
  PROPERTIES
  OUTPUT_NAME holepunch
)

target_link_libraries(
  holepunch_static
  PUBLIC
    uv
  PRIVATE
    archive_static
    path_static
)

install(TARGETS holepunch_static)

install(FILES include/holepunch.h DESTINATION include)
install(DIRECTORY include/holepunch DESTINATION include)

if(PROJECT_IS_TOP_LEVEL)
  enable_testing()
  add_subdirectory(test)
endif()
