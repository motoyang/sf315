cmake_minimum_required(VERSION 2.8)

project(rublk)

ADD_DEFINITIONS(-DSTBI_NO_DDS)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_BUILD_TYPE "Debug")
#set(CMAKE_BUILD_TYPE "Release")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -stdlib=libc++")
include_directories(~/.local/include/c++/v1)
link_directories(~/.local/lib/)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -v")

set(SRC_GLAD_C "glad/glad.c")
set(SRC_STBIMAGE_C "stb/stb_image_aug.c")

#查找当前文件夹中的所有源代码文件，也可以通过Set命令将所有文件设置为一个变量
FILE(GLOB SRC_FILES "./*.cpp")
#同理，查找当前文件夹中所有的头文件
FILE(GLOB HEAD_FILES "./*.h")
#同理，查找所有的vs和fs文件
FILE(GLOB SHADER_FILES "shader/*.vs" "shader/*.fs")

add_executable(${PROJECT_NAME}
  ${SRC_GLAD_C}
  ${SRC_STBIMAGE_C}
  ${SRC_FILES}
  ${SHADER_FILES}
  )
target_link_libraries(${PROJECT_NAME}
  dl
  GL
  glfw
  )
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
