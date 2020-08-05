# verilogAST-cpp
[![Build Status](https://travis-ci.com/leonardt/verilogAST-cpp.svg?branch=master)](https://travis-ci.com/leonardt/verilogAST-cpp)
[![Coverage Status](https://coveralls.io/repos/github/leonardt/verilogAST-cpp/badge.svg)](https://coveralls.io/github/leonardt/verilogAST-cpp)

C++17 Implementation of an AST for Verilog code generation

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
