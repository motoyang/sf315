cmake_minimum_required(VERSION 3.8)

#设置工程名称
project(gof)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# set(CMAKE_PREFIX_PATH "/Users/xt/Qt/5.12.1/clang_64")
# set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-inconsistent-missing-override")

# set(CMAKE_BUILD_TYPE "Debug")
#set(CMAKE_BUILD_TYPE "Release")

#查找当前文件夹中的所有源代码文件，也可以通过Set命令将所有文件设置为一个变量
# FILE(GLOB SRC_FILES "./*.cpp")
#同理，查找当前文件夹中所有的头文件
# FILE(GLOB HEAD_FILES "./*.h")

include_directories(
  "/usr/local/include"
  "${PROJECT_SOURCE_DIR}/include"
  )

# 添加子目录（每个子目录下都必须有自已的CMakeLists.txt文件
add_subdirectory(test)
add_subdirectory(examples)
