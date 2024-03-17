set(CMAKE_CXX_STANDARD 14)

add_definitions(-pthread)

include_directories($ENV{XILINX_XRT}/include)

add_library(hw STATIC hw.cpp)