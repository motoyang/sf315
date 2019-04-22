# project(uvplus-test)

# 设置生成的库文件目录
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/test/bin)
# 设置可执行文件目录
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/test/bin)

include_directories(
)

set(UV_LIB "libuv.a")
set(PTHREAD_LIB "pthread")

set(CMAKE_BUILD_TYPE "Debug")
# set(CMAKE_BUILD_TYPE "Release")

macro (test_build arg1)
  file(GLOB EXAMPLE_SRC_FILES "${arg1}/*.cpp")
  add_executable("test_${arg1}" ${EXAMPLE_SRC_FILES})
  target_link_libraries("test_${arg1}" ${UV_LIB} ${PTHREAD_LIB})
endmacro (test_build)

#test_build(uvplus)
#test_build(uvp)

# --

macro (test_botan arg1)
  file(GLOB EXAMPLE_SRC_FILES "${arg1}/*.cpp")
  add_executable("test_${arg1}" ${EXAMPLE_SRC_FILES})
  target_link_libraries("test_${arg1}" "botan-2")
endmacro (test_botan)

# test_botan(botan_1)