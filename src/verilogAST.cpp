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
  case BINARY:
    radix_str = 'b';
    break;
  case OCTAL:
    radix_str = 'o';
    break;
  case HEX:
    radix_str = 'h';
    break;
  case DECIMAL:
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

std::string BinaryOp::toString() {
  std::string op_str;
  switch (op) {
  case LSHIFT:
    op_str = "<<";
    break;
  case RSHIFT:
    op_str = ">>";
    break;
  case _AND:
    op_str = "&&";
    break;
  case _OR:
    op_str = "||";
    break;
  case EQ:
    op_str = "==";
    break;
  case NEQ:
    op_str = "!=";
    break;
  case ADD:
    op_str = "+";
    break;
  case SUB:
    op_str = "-";
    break;
  case MUL:
    op_str = "*";
    break;
  case DIV:
    op_str = "/";
    break;
  case POW:
    op_str = "**";
    break;
  case MOD:
    op_str = "%";
    break;
  case ALSHIFT:
    op_str = "<<<";
    break;
  case ARSHIFT:
    op_str = ">>>";
    break;
  }
  return left->toString() + ' ' + op_str + ' ' + right->toString();
};

}; // namespace verilogAST
