cmake_minimum_required (VERSION 3.5)
project (quant_circ C CXX)

set(CPP_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)

link_directories( "${PROJECT_SOURCE_DIR}/Quest_")
include_directories("${PROJECT_SOURCE_DIR}/QuEST_/include/" "${PROJECT_SOURCE_DIR}/metis-5.1.0/include")

add_library(quant_circ_lib
    ${CPP_SOURCE_DIR}/graph_partition.cpp
    ${CPP_SOURCE_DIR}/parser.cpp
    ${CPP_SOURCE_DIR}/quest_test.cpp
    ${CPP_SOURCE_DIR}/simulate.cpp
    ${CPP_SOURCE_DIR}/tensor_network.cpp
    ${CPP_SOURCE_DIR}/genetic_partitioning.cpp
)
add_executable(test_quant_circ
    ${CPP_SOURCE_DIR}/quest_test.cpp
)
add_executable(view_quant_circ
    ${CPP_SOURCE_DIR}/view_decomposition.cpp
)
add_executable(compare_partitionings
    ${CPP_SOURCE_DIR}/compare_partitionings.cpp
)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set_property(TARGET quant_circ_lib PROPERTY CXX_STANDARD 14)
set_property(TARGET test_quant_circ PROPERTY CXX_STANDARD 14)
set_property(TARGET view_quant_circ PROPERTY CXX_STANDARD 14)
set_property(TARGET compare_partitionings PROPERTY CXX_STANDARD 14)

set(MULTITHREADED FALSE CACHE BOOL "disable multithreading for quest")

add_subdirectory("metis-5.1.0")
add_subdirectory("QuEST_")
#add_subdirectory("qtorch")

target_link_libraries(quant_circ_lib PUBLIC QuEST metis)
target_link_libraries(quant_circ_lib PUBLIC "${OpenMP_CXX_FLAGS}")
target_link_libraries(test_quant_circ quant_circ_lib Threads::Threads)
target_link_libraries(view_quant_circ quant_circ_lib Threads::Threads)
target_link_libraries(compare_partitionings quant_circ_lib Threads::Threads)
