set(CMAKE_CXX_STANDARD 11)

add_definitions(-DCURVE=BN128)

include_directories(/usr/local/include)
include_directories(../libsnark)
include_directories(../libsnark/depends/libfqfft/)
link_directories(/usr/local/lib)
link_directories($ENV{XILINX_XRT}/lib)

add_executable(merkle main.cpp ../circuit/merklecircuit.h ../hw/hw.h)

target_link_libraries(merkle ff)
target_link_libraries(merkle gmpxx)
target_link_libraries(merkle gmp)
target_link_libraries(merkle procps)
target_link_libraries(merkle hw)
target_link_libraries(merkle xrt_coreutil)
