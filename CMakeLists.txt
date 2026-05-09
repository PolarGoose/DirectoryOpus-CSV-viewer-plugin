cmake_minimum_required(VERSION 3.31)
project(DirectoryOpus-CSV-viewer-plugin LANGUAGES CXX)
include(FetchContent)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(VERSION_MAJOR 0 CACHE STRING "Major version")
set(VERSION_MINOR 0 CACHE STRING "Minor version")

find_package(Boost REQUIRED COMPONENTS utility)
find_package(Boost REQUIRED COMPONENTS locale)
find_package(Boost REQUIRED COMPONENTS system)
find_path(WTL_INCLUDE_DIRS "atlapp.h")

# Download DirectoryOpus Plugin API
FetchContent_Populate(
  directory_opus_plugin_sdk
  URL https://cdn2.gpsoft.com.au/files/Misc/opus_sdk.zip
  URL_HASH MD5=a244c8d64f2211b3bf2774bd8b997c6d)
file(COPY "${directory_opus_plugin_sdk_SOURCE_DIR}/headers/" DESTINATION "${CMAKE_BINARY_DIR}/DirectoryOpus-plugin-SDK/DirectoryOpus-plugin-SDK")

file(DOWNLOAD
  https://github.com/vincentlaucsb/csv-parser/releases/download/5.1.0/csv.hpp
  "${CMAKE_BINARY_DIR}/vincentlaucsb_csv_parser/vincentlaucsb/csv.hpp"
  EXPECTED_HASH SHA256=90a9b032dcde3f38c1baac5f873a09fc6a41e63d972fb810f1bfccdf5c8040a8)

# Configure DirectoryOpus-CSV-viewer-plugin target
file(GLOB_RECURSE src_files CONFIGURE_DEPENDS
  "src/DirectoryOpus-CSV-viewer-plugin/*.h"
  "src/DirectoryOpus-CSV-viewer-plugin/*.cpp"
  "src/DirectoryOpus-CSV-viewer-plugin/*.rc")
add_library(DirectoryOpus-CSV-viewer-plugin MODULE ${src_files})
target_include_directories(DirectoryOpus-CSV-viewer-plugin PRIVATE
  "src"
  "${CMAKE_BINARY_DIR}/DirectoryOpus-plugin-SDK"
  "${CMAKE_BINARY_DIR}/vincentlaucsb_csv_parser"
  ${WTL_INCLUDE_DIRS})
target_compile_features(DirectoryOpus-CSV-viewer-plugin PUBLIC cxx_std_23)
target_compile_options(DirectoryOpus-CSV-viewer-plugin PUBLIC
  /utf-8
  /external:anglebrackets
  /external:W0
  /WX
  /W4
  /analyze
  /analyze:external-
  $<$<CONFIG:Release>:/GL>)
target_compile_definitions(DirectoryOpus-CSV-viewer-plugin PUBLIC
  UNICODE
  DOPUS_PLUGIN_HELPER
  WIN32_LEAN_AND_MEAN
  VC_EXTRALEAN
  NOMINMAX
  VERSION_MAJOR=${VERSION_MAJOR}
  VERSION_MINOR=${VERSION_MINOR}
  $<$<CONFIG:Debug>:DEBUG>)
target_link_libraries(DirectoryOpus-CSV-viewer-plugin PRIVATE Boost::utility Boost::locale Boost::system)
target_precompile_headers(DirectoryOpus-CSV-viewer-plugin PRIVATE src/DirectoryOpus-CSV-viewer-plugin/PrecompiledHeader.h)
