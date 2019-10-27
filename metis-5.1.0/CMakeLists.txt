cmake_minimum_required(VERSION 2.8)
project(METIS)

set(GKLIB_PATH "${CMAKE_CURRENT_LIST_DIR}/GKlib" CACHE PATH "path to GKlib")
set(SHARED FALSE CACHE BOOL "build a shared library")

# Configure libmetis library.
set(METIS_LIBRARY_TYPE STATIC)

include(${GKLIB_PATH}/GKlibSystem.cmake)
# Add include directories.
include_directories(${GKLIB_PATH})
include_directories(include)
# Recursively look for CMakeLists.txt in subdirs.
add_subdirectory("libmetis")
