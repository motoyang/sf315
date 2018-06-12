
cmake_minimum_required(VERSION 2.8.12)

# CMake policies. 0025 needs to be before project(), so putting all there.
# Use AppleClang instead of Clang on Apple
if(POLICY CMP0025)
    cmake_policy(SET CMP0025 NEW)
endif()
# Don't treat imported targets with :: as files
if(POLICY CMP0028)
    cmake_policy(SET CMP0028 NEW)
endif()
# Enable MACOSX_RPATH by default
if(POLICY CMP0042)
    cmake_policy(SET CMP0042 NEW)
endif()
# Quoted variables should not be dereferenced
if(POLICY CMP0054)
    cmake_policy(SET CMP0054 NEW)
endif()

project(MagnumRublk)

set(CMAKE_BUILD_TYPE "Debug")
#set(CMAKE_BUILD_TYPE "Release")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -v")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -stdlib=libc++ -v")
#include_directories(~/.local/include/c++/v1)
#link_directories(~/.local/lib/)

# Add module path in case this is project root
if(PROJECT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/modules/")
endif()

#find_package(Corrade REQUIRED Containers)
find_package(Magnum REQUIRED GL Trade Sdl2Application Shaders SceneGraph)

set_directory_properties(PROPERTIES CORRADE_USE_PEDANTIC_FLAGS ON)

corrade_add_resource(MagnumRublk_RESOURCES resources.conf)

#查找当前文件夹中的所有源代码文件，也可以通过Set命令将所有文件设置为一个变量
FILE(GLOB SRC_FILES "./*.cpp")
#同理，查找当前文件夹中所有的头文件
FILE(GLOB HEAD_FILES "./*.h")

add_executable(magnum-rublk
    ${SRC_FILES}
    ${HEAD_FILES}
    ${MagnumRublk_RESOURCES})
target_link_libraries(magnum-rublk PRIVATE
    Magnum::Application
    Magnum::GL
    Magnum::Magnum
    Magnum::Trade
    Magnum::SceneGraph
    Magnum::Shaders
    )

install(TARGETS magnum-rublk DESTINATION ${MAGNUM_BINARY_INSTALL_DIR})
