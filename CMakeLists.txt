cmake_minimum_required(VERSION 3.21.1)
project(rix)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

#Set your project compile flags.
#E.g.if using the C++ header files
#you will need to enable C++ 11 support
#for your compiler.

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST}) 

file(GLOB_RECURSE SOURCES ./source/*.cpp ./source/json.hpp)

set(CMAKE_CXX_STANDARD 23)

add_executable(bin-rix ${SOURCES})

#Find the libraries that correspond to the LLVM components
#that we wish to use
llvm_map_components_to_libnames(llvm_libs support core irreader)

#Link against LLVM libraries
target_link_libraries(bin-rix ${llvm_libs})