# verilogAST-cpp
[![Build Status](https://travis-ci.com/leonardt/verilogAST-cpp.svg?branch=master)](https://travis-ci.com/leonardt/verilogAST-cpp)
[![Coverage Status](https://coveralls.io/repos/github/leonardt/verilogAST-cpp/badge.svg)](https://coveralls.io/github/leonardt/verilogAST-cpp)

C++17 Implementation of an AST for Verilog

## Installation
### Linux
```
curl -s -L https://github.com/leonardt/verilogAST-cpp/releases/latest | grep "href.*libverilogAST-linux.tar.gz" | cut -d \" -f 2 | xargs -I {} wget https://github.com"{}"
tar -xf libverilogAST-linux.tar.gz
cd libverilogAST-linux 
make install
```
### OSX
```
curl -s -L https://github.com/leonardt/verilogAST-cpp/releases/latest | grep "href.*libverilogAST-osx.tar.gz" | cut -d \" -f 2 | xargs -I {} wget https://github.com"{}"
tar -xf libverilogAST-osx.tar.gz
cd libverilogAST-osx 
make install
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
