cmake_minimum_required(VERSION 2.8)

cmake_policy(SET CMP0071 OLD)

#设置工程名称
project(luaplot)

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

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX17_EXTENSION_COMPILE_OPTION "-std=c++17")
set(CMAKE_CXX17_STANDARD_COMPILE_OPTION "-std=c++17")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -v -Wno-inconsistent-missing-override")

#设置工程包含当前目录，非必须
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_BUILD_TYPE "Debug")
#set(CMAKE_BUILD_TYPE "Release")

# cmake在生成链接文件时，会自动加上-rdyamic, 加入以下两句清空动态链接的选项
#set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS " ")
#set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS " ")

#set(CMAKE_LINK_LIBRARY_FLAG "-Wl,--as-needed ")

#设置自动生成moc文件，一定要设置
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

#查找需要的Qt库文件，最好每一个库都要写，Qt也会根据依赖关系自动添加
find_package(Qt5Widgets)
find_package(Qt5Core)
find_package(Qt5Gui)
find_package(Qt5PrintSupport)

#查找当前文件夹中的所有源代码文件，也可以通过Set命令将所有文件设置为一个变量
FILE(GLOB SRC_FILES "./*.cpp" "./qcp/*.cpp")
#同理，查找当前文件夹中所有的头文件
FILE(GLOB HEAD_FILES "./*.h" "./qcp/*.h")
#同理，查找当前文件夹中所有的ui文件
FILE(GLOB UI_FILES "./*.ui")
#同理，查找当前文件夹中所有的qrc文件
FILE(GLOB QRC_FILES "./*.qrc")
FILE(GLOB VER_FILE "./ver.cc")

#通过Ui文件生成对应的头文件，一定要添加
#qt5_wrap_cpp(MOC ${HEAD_FILES})
qt5_wrap_ui(WRAP_FILES ${UI_FILES})
#qt5_add_resources(RCC ${QRC_FILES})

include_directories(
#    ${PROJECT_SOURCE_DIR}/pp
#    ${PROJECT_SOURCE_DIR}/qcp
    )

# Add block directories
#add_subdirectory(qcp)

set (VER_LIB "_Version")
add_library(${VER_LIB} ${VER_FILE})

#创建工程文件
add_executable(${PROJECT_NAME}
    ${SRC_FILES}
    ${QRC_FILES}
    ${WRAP_FILES}
    )

set (LUA_LIB "lua-5.3")
target_link_libraries(${PROJECT_NAME}
    ${LUA_LIB}
    ${VER_LIB}
    Qt5::Widgets Qt5::Core Qt5::Gui Qt5::PrintSupport
    )
#link_directories(/lib/x86_64-linux-gnu)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lua)

add_custom_command(TARGET ${VER_LIB}
  PRE_BUILD
  COMMAND echo "daily build command"
  COMMAND /home/xt/bin/ver.sh ver.h
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  COMMENT "This command will be executed before building target ${VER_LIB}"
  )
