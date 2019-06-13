#include "verilogAST.hpp"

namespace verilogAST {
std::string NumericLiteral::toString() {
  std::string signed_str = _signed ? "s" : "";

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
  return std::to_string(size) + "'" + signed_str + radix_str + value;
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
    case BinOp::LSHIFT:
      op_str = "<<";
      break;
    case BinOp::RSHIFT:
      op_str = ">>";
      break;
    case BinOp::AND:
      op_str = "&&";
      break;
    case BinOp::OR:
      op_str = "||";
      break;
    case BinOp::EQ:
      op_str = "==";
      break;
    case BinOp::NEQ:
      op_str = "!=";
      break;
    case BinOp::ADD:
      op_str = "+";
      break;
    case BinOp::SUB:
      op_str = "-";
      break;
    case BinOp::MUL:
      op_str = "*";
      break;
    case BinOp::DIV:
      op_str = "/";
      break;
    case BinOp::POW:
      op_str = "**";
      break;
    case BinOp::MOD:
      op_str = "%";
      break;
    case BinOp::ALSHIFT:
      op_str = "<<<";
      break;
    case BinOp::ARSHIFT:
      op_str = ">>>";
      break;
  }
  return left->toString() + ' ' + op_str + ' ' + right->toString();
};

std::string UnaryOp::toString() {
  std::string op_str;
  switch (op) {
    case UnOp::NOT:
      op_str = "!";
      break;
    case UnOp::INVERT:
      op_str = "~";
      break;
    case UnOp::AND:
      op_str = "&";
      break;
    case UnOp::NAND:
      op_str = "~&";
      break;
    case UnOp::OR:
      op_str = "|";
      break;
    case UnOp::NOR:
      op_str = "~|";
      break;
    case UnOp::XOR:
      op_str = "^";
      break;
    case UnOp::NXOR:
      op_str = "~^";
      break;
    case UnOp::XNOR:
      op_str = "^~";
      break;
    case UnOp::PLUS:
      op_str = "+";
      break;
    case UnOp::MINUS:
      op_str = "-";
      break;
  }
  return op_str + ' ' + operand->toString();
};

std::string TernaryOp::toString() {
  return cond->toString() + " ? " + true_value->toString() + " : " +
         false_value->toString();
}

std::string NegEdge::toString() { return "negedge " + value->toString(); }

std::string PosEdge::toString() { return "posedge " + value->toString(); }

template <typename... Ts>
std::string variant_to_string(std::variant<Ts...> value) {
  return std::visit(
      [](auto &&value) -> std::string { return value->toString(); }, value);
}

std::string Port::toString() {
  std::string value_str =
      variant_to_string<Identifier *, Index *, Slice *>(value);
  std::string direction_str;
  switch (direction) {
    case INPUT:
      direction_str = "input";
      break;
    case OUTPUT:
      direction_str = "output";
      break;
    case INOUT:
      direction_str = "inout";
      break;
  }

  std::string data_type_str;
  switch (data_type) {
    case WIRE:
      data_type_str = "";
      break;
    case REG:
      data_type_str = "reg ";
      break;
  }
  return direction_str + " " + data_type_str + value_str;
}

std::string join(std::vector<std::string> vec, std::string separator) {
  std::string result;
  for (size_t i = 0; i < vec.size(); i++) {
    if (i > 0) result += separator;
    result += vec[i];
  }
  return result;
}

std::string Module::toString() {
  std::string module_str = "";
  module_str += "module " + name;

  // emit parameter string
  if (!parameters.empty()) {
    module_str += " #(";
    std::vector<std::string> param_strs;
    for (auto it : parameters) {
      param_strs.push_back("parameter " + it.first + " = " +
                           it.second->toString());
    }
    module_str += join(param_strs, ", ");
    module_str += ")";
  }

  // emit port string
  module_str += " (";
  std::vector<std::string> ports_strs;
  for (auto it : ports) ports_strs.push_back(it->toString());
  module_str += join(ports_strs, ", ");
  module_str += ");\n";

  // emit body
  for (auto statement : body) {
    module_str +=
        variant_to_string<Always *, StructuralStatement *, Declaration *>(
            statement) +
        ";\n";
  }

  module_str += "endmodule\n";
  return module_str;
}

std::string ModuleInstantiation::toString() {
  std::string module_inst_str = "";
  module_inst_str += module_name;
  if (!parameters.empty()) {
    module_inst_str += " #(";
    std::vector<std::string> param_strs;
    for (auto it : parameters) {
      param_strs.push_back("." + it.first + "(" + it.second->toString() + ")");
    }
    module_inst_str += join(param_strs, ", ");
    module_inst_str += ")";
  }
  module_inst_str += " " + instance_name + "(";
  if (!connections.empty()) {
    std::vector<std::string> param_strs;
    for (auto it : connections) {
      param_strs.push_back("." + it.first + "(" + variant_to_string(it.second) +
                           ")");
    }
    module_inst_str += join(param_strs, ", ");
  }
  module_inst_str += ")";
  return module_inst_str;
}

std::string Declaration::toString() {
  return decl + " " + variant_to_string(value);
}

std::string Assign::toString() {
  return prefix + variant_to_string(target) + " = " + value->toString();
}

};  // namespace verilogAST
