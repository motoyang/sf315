cmake_minimum_required(VERSION 2.8)

project(nanolog)

# ON is the default
option(USE_CLANG "build application with clang" ON)

if(USE_CLANG)
  # begin of clang settings
  set(CMAKE_C_COMPILER "clang")
  set(CMAKE_CXX_COMPILER "clang++")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -stdlib=libc++")
  include_directories(~/.local/include/c++/v1)
  link_directories(~/.local/lib/)
  # end of clang
endif()

set(CMAKE_BUILD_TYPE "Debug")
#set(CMAKE_BUILD_TYPE "Release")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -v")
# cmake在生成链接文件时，会自动加上-rdyamic, 加入以下两句清空动态链接的选项
# set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS " ")
# set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "-Wl,--as-needed")

set(SRC_FILES "main.cpp")
add_executable(${PROJECT_NAME} ${SRC_FILES})
target_link_libraries(${PROJECT_NAME} pthread)

