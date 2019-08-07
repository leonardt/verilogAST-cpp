UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S), Linux)
SUFFIX=so
PREFIX?=/usr
endif
ifeq ($(UNAME_S), Darwin)
SUFFIX=dylib
PREFIX?=/usr/local
endif

install:
	install verilogAST.hpp $(PREFIX)/include
	install libverilogAST.$(SUFFIX) $(PREFIX)/lib

uninstall:
	-rm $(PREFIX)/include/verilogAST.hpp
	-rm $(PREFIX)/lib/libverilogAST.$(SUFFIX) 

.PHONY: install uninstall
