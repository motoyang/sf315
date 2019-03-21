# project(gof-examples)

# 设置生成的库文件目录
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
# 设置可执行文件目录
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

set(CMAKE_BUILD_TYPE "Debug")
# set(CMAKE_BUILD_TYPE "Release")

# set(UV_LIB "libuv.a")
set(LIB_NEEDED "pthread")

macro (example_build arg1)
  file(GLOB EXAMPLE_SRC_FILES "${arg1}/*.cpp")
  add_executable(${arg1} ${EXAMPLE_SRC_FILES})
#  target_link_libraries(${arg1} ${UV_LIB} ${PTHREAD_LIB})
endmacro (example_build)

macro (example_build2 arg1 arg2)
  file(GLOB EXAMPLE_SRC_FILES "${arg1}/*.cpp")
  add_executable(${arg1} ${EXAMPLE_SRC_FILES})
  target_link_libraries(${arg1} ${arg2})
endmacro (example_build2)

example_build(factory)
example_build(visitor)
example_build(visit)
example_build2(parallel ${LIB_NEEDED})
