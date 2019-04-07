# project(tls13-examples)

# 设置生成的库文件目录
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/examples/bin)
# 设置可执行文件目录
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/examples/bin)

set(CMAKE_BUILD_TYPE "Debug")
# set(CMAKE_BUILD_TYPE "Release")

set(BOTAN_LIB "botan-2")
set(PTHREAD_LIB "pthread")

file(GLOB TLS13_SRC_FILES "../src/*.cpp")

macro (example_build arg1)
  set(TEST_TARGET "${arg1}")
  file(GLOB TEST_SRC_FILES "${arg1}/*.cpp")
  add_executable(${TEST_TARGET} ${TEST_SRC_FILES} ${TLS13_SRC_FILES})
  target_link_libraries(${TEST_TARGET} ${BOTAN_LIB} ${PTHREAD_LIB})
endmacro (example_build)

example_build(one)
# example_build(client)
# example_build(server)
