cmake_minimum_required(VERSION 3.0)
project(mprpc)

# debug version
set(CMAKE_BUILD_TYPE "Debug")
# set(CMAKE_VERBOSE_MAKEFILE on)

# bin output path
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
# lib output path
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# header path: g++ -I
include_directories(${PROJECT_SOURCE_DIR}/src/include)
include_directories(${PROJECT_SOURCE_DIR}/example)
include_directories(/home/coder/build/release-install-cpp11/include)    # muduo
# lib path: g++ -l
link_directories(${PROJECT_SOURCE_DIR}/lib)
link_directories(/home/coder/build/release-install-cpp11/lib)   # muduo

add_subdirectory(src)
add_subdirectory(example)
