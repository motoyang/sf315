#设置cmake版本号  
cmake_minimum_required(VERSION 3.4.0)  

cmake_policy(SET CMP0071 OLD)

#设置工程名称  
project(uv_1)  

# The version number.
set (VERSION_MAJOR 0)
set (VERSION_MINOR 1)
set (VERSION_PATCH 0)
 
configure_file (
  "${PROJECT_SOURCE_DIR}/ver.h.in"
  "${PROJECT_BINARY_DIR}/ver.h"
  )
include_directories ("${PROJECT_BINARY_DIR}")
 
#设置工程包含当前目录，非必须  
# set(CMAKE_INCLUDE_CURRENT_DIR ON)   

# ON is the default
option(USE_CLANG "build application with clang" ON)
if(USE_CLANG)
  # begin of clang settings
  set(CMAKE_C_COMPILER "clang")
  set(CMAKE_CXX_COMPILER "clang++")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
  include_directories(~/.local/include/c++/v1 /usr/local/include)
  link_directories(~/.local/lib/)
  # end of clang
endif()

include_directories(
  # ${PROJECT_SOURCE_DIR}/../msgpack-c/include
  ${PROJECT_SOURCE_DIR}/uv
  ${PROJECT_SOURCE_DIR}/uvp
  ${PROJECT_SOURCE_DIR}/uvp/msgpack-c
  )

#查找当前文件夹中的所有源代码文件，也可以通过Set命令将所有文件设置为一个变量  
FILE(GLOB SRC_FILES "*.cpp")   
FILE(GLOB CPP_FILES "uvp/*.cpp")   
#同理，查找当前文件夹中所有的头文件  
FILE(GLOB HEAD_FILES "./*.h")   
FILE(GLOB HPP_FILES "./*.hpp")   
  
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-inconsistent-missing-override -Wno-braced-scalar-init")

#创建工程文件  
add_executable(${PROJECT_NAME} ${SRC_FILES} ${CPP_FILES} ${HEAD_FILES} ${HPP_FILES})  
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)  

set (UV_LIB "${CMAKE_SOURCE_DIR}/lib/libuv.so")
set (PTHREAD_LIB "pthread")
target_link_libraries(${PROJECT_NAME}
    ${UV_LIB}
    ${PTHREAD_LIB}
    )

set (VER_TARGET "_Version")
add_custom_target(${VER_TARGET} ALL)
add_dependencies(${PROJECT_NAME} ${VER_TARGET})
add_custom_command(TARGET ${VER_TARGET}
  POST_BUILD
  COMMAND echo "daily build command."
  COMMAND $ENV{HOME}/bin/ver.sh ver.h.in
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  COMMENT "This command will be executed before building the project."
  )
