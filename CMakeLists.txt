cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 17)

project(verilogAST)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib") 
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

include_directories(
  ${PROJECT_SOURCE_DIR}/include/
) 

set(LIB_SOURCES src/verilogAST.cpp)

set(LIBRARY_NAME verilogAST)
add_library(${LIBRARY_NAME} SHARED ${LIB_SOURCES})
