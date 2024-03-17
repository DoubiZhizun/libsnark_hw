cmake_minimum_required(VERSION 3.0)

project(merkle)
set(CMAKE_BUILD_TYPE "Release")

option(
  MULTICORE
  "Enable parallelized execution, using OpenMP"
  ON
)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
add_definitions(-DMULTICORE=1)

add_subdirectory(merkle)
add_subdirectory(hw)
add_subdirectory(libsnark)
