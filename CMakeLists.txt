cmake_minimum_required (VERSION 3.5)
project (quant_circ C CXX)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(CPP_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)

link_directories( "${PROJECT_SOURCE_DIR}/Quest_")
include_directories("${PROJECT_SOURCE_DIR}/QuEST_/include/" "${PROJECT_SOURCE_DIR}/metis-5.1.0/include")

add_executable(quant_circ
    ${CPP_SOURCE_DIR}/graph_partition.cpp
    ${CPP_SOURCE_DIR}/parser.cpp
    ${CPP_SOURCE_DIR}/quest_test.cpp
    ${CPP_SOURCE_DIR}/simulate.cpp
    ${CPP_SOURCE_DIR}/tensor_network.cpp
)

set_property(TARGET quant_circ PROPERTY CXX_STANDARD 14)
set_property(TARGET quant_circ PROPERTY CXX_STANDARD_REQUIRED ON)

add_subdirectory("metis-5.1.0")
add_subdirectory("QuEST_")

target_link_libraries(quant_circ PRIVATE metis QuEST)
target_link_libraries(quant_circ PRIVATE "${OpenMP_CXX_FLAGS}")
