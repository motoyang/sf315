#设置cmake版本号
cmake_minimum_required(VERSION 3.8.0)

# cmake_policy(SET CMP0071 OLD)

#设置工程名称
project(uvsecure)

# The version number.
set (VERSION_MAJOR 0)
set (VERSION_MINOR 1)
set (VERSION_PATCH 0)
configure_file (
  "${PROJECT_SOURCE_DIR}/ver.h.in"
  "${PROJECT_BINARY_DIR}/ver.h"
  )
include_directories ("${PROJECT_BINARY_DIR}")

# updated VERSION_BUILD pre build
set (VER_TARGET "_Version")
add_custom_target(${VER_TARGET} ALL)
add_custom_command(TARGET ${VER_TARGET}
  PRE_BUILD
#  COMMAND $ENV{HOME}/bin/ver.sh ver.h.in
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  COMMENT "This command will be executed before building the project."
  )

# --

# which compiler kits will be used?
if (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
  message("Using clang++ as c++ compiler.")

  include_directories("~/.local/include/c++/v1")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
  link_directories(~/.local/lib)
elseif(${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
  message("Using g++ as c++ compiler.")

elseif(${CMAKE_CXX_COMPILER_ID} MATCHES "MSVC")
  message("Using msvc as c++ compiler.")

elseif(${CMAKE_CXX_COMPILER_ID} MATCHES "AppleClang")
  message("Using AppleClang++ as c++ compiler.")

endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --

set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(
  /usr/local/include/botan-2
  ${PROJECT_SOURCE_DIR}/uv
  ${PROJECT_SOURCE_DIR}/include
  )

link_directories(
  ${PROJECT_SOURCE_DIR}/libs
#  /usr/local/lib
  )

set(CMAKE_CXX_STANDARD 17)

# 添加子目录（每个子目录下都必须有自已的CMakeLists.txt文件
add_subdirectory(uvsecure-examples)
add_subdirectory(test)

