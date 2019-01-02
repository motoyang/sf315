#设置cmake版本号  
cmake_minimum_required(VERSION 3.4.0)  

cmake_policy(SET CMP0071 OLD)

#设置工程名称  
project(uvplus)  

# The version number.
set (VERSION_MAJOR 0)
set (VERSION_MINOR 1)
set (VERSION_PATCH 0)
 
configure_file (
  "${PROJECT_SOURCE_DIR}/ver.h.in"
  "${PROJECT_BINARY_DIR}/ver.h"
  )
include_directories ("${PROJECT_BINARY_DIR}")
 
# ON is the default
option(USE_CLANG "build application with clang" ON)
if(USE_CLANG)
  # begin of clang settings
  # set(CMAKE_C_COMPILER "clang")
  # set(CMAKE_CXX_COMPILER "/home/xt/.local/bin/clang++")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
  include_directories(~/.local/include/c++/v1 /usr/local/include)
  link_directories(~/.local/lib/)
  # end of clang
endif()

include_directories(
  ${PROJECT_SOURCE_DIR}/uv
  ${PROJECT_SOURCE_DIR}/msgpack-c
  ${PROJECT_SOURCE_DIR}/include
  )

link_directories(${PROJECT_SOURCE_DIR}/libs)

set(CMAKE_CXX_STANDARD 17)

# 添加子目录（每个子目录下都必须有自已的CMakeLists.txt文件
add_subdirectory(uvbook-examples)
add_subdirectory(uvplus-examples)

set (VER_TARGET "_Version")
add_custom_target(${VER_TARGET} ALL)
add_custom_command(TARGET ${VER_TARGET}
  PRE_BUILD
  COMMAND $ENV{HOME}/bin/ver.sh ver.h.in
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  COMMENT "This command will be executed before building the project."
  )
