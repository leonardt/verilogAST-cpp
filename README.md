# verilogAST-cpp
[![Build Status](https://travis-ci.com/leonardt/verilogAST-cpp.svg?branch=master)](https://travis-ci.com/leonardt/verilogAST-cpp)

C++17 Implementation of an AST for Verilog

## Dependencies
* Compiler: Tested using gcc-7 on Ubuntu trusty (14.04.5) and Xcode 10.2.1 on macOS 10.14.

## Building
```
mkdir build
cd build
cmake ..
make
```

## Testing
```
cd build
make test
```

## Style
All changes should be processed using `clang-format` before merging into
master.
