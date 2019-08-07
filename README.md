# verilogAST-cpp
[![Build Status](https://travis-ci.com/leonardt/verilogAST-cpp.svg?branch=master)](https://travis-ci.com/leonardt/verilogAST-cpp)
[![Coverage Status](https://coveralls.io/repos/github/leonardt/verilogAST-cpp/badge.svg)](https://coveralls.io/github/leonardt/verilogAST-cpp)

C++17 Implementation of an AST for Verilog

## Installation
### Linux
```
wget https://github.com/leonardt/verilogAST-cpp/releases/download/0.0.7/libverilogAST-linux.tar.gz
tar -xf libverilogAST-linux.tar.gz
cd libverilogAST-linux 
sudo ./verilogAST-0.0.7-Linux.sh 
```
### OSX
```
wget https://github.com/leonardt/verilogAST-cpp/releases/download/0.0.7/libverilogAST-osx.tar.gz
tar -xf libverilogAST-osx.tar.gz
cd libverilogAST-osx 
./verilogAST-0.0.7-Darwin.sh
```

## Dependencies
* Compiler: Tested using gcc-7 on Ubuntu trusty (14.04.5) and Xcode 10.2.1 on macOS 10.14.

## Building
```
mkdir build
cd build
cmake ..
cmake --build .
```

## Testing
```
# inside build directory
cmake -DVERILOGAST_BUILD_TESTS=ON ..
cmake --build .
ctest
```

## Style
All changes should be processed using `clang-format` before merging into
master.
