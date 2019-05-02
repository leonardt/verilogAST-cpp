#include "verilogAST.hpp"
namespace verilogAST {
std::string NumericLiteral::toString() {
  if (_signed) {
    throw std::runtime_error(
        "NumericLiteral::toString when _signed == True has "
        "not been implemented");
  }
  char radix_str;
  switch (radix) {
  case binary:
    radix_str = 'b';
    break;
  case octal:
    radix_str = 'o';
    break;
  case hex:
    radix_str = 'h';
    break;
  case decimal:
    radix_str = 'd';
    break;
  }
  return std::to_string(size) + "'" + radix_str + value;
};

std::string Identifier::toString() { return value; };

std::string Index::toString() {
  return id->toString() + '[' + index->toString() + ']';
};

std::string Slice::toString() {
  return id->toString() + '[' + high_index->toString() + ':' +
         low_index->toString() + ']';
};

}; // namespace verilogAST
