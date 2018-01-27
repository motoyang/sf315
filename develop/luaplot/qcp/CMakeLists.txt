cmake_minimum_required(VERSION 2.8)

find_package(Qt5Core)

aux_source_directory(. QCP_FILES)

set (QCP_LIB "_qcplot")
add_library (${QCP_LIB} ${QCP_FILES})
target_link_libraries(${QCP_LIB}
    Qt5::Core
    )

#set (LIBRARY_OUTPUT_PATH <路径>)