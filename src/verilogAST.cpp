#include "verilogAST.hpp"

#include <regex>
#include <unordered_set>

// Helper function to join a vector of strings with a specified separator ala
// Python's ",".join(...)
std::string join(std::vector<std::string> vec, std::string separator) {
  std::string result;
  for (size_t i = 0; i < vec.size(); i++) {
    if (i > 0) result += separator;
    result += vec[i];
  }
  return result;
}

namespace verilogAST {
std::string NumericLiteral::toString() {
  std::string signed_str = _signed ? "s" : "";

  std::string radix_str;
  switch (radix) {
    case BINARY:
      radix_str = "b";
      break;
    case OCTAL:
      radix_str = "o";
      break;
    case HEX:
      radix_str = "h";
      break;
    case DECIMAL:
      radix_str = "";
      break;
  }
  std::string size_str = std::to_string(size);
  if (size_str == "32") {
    size_str = "";
  }

  std::string separator = "";
  if (size_str + signed_str + radix_str != "") {
    separator = "'";
  }
  return size_str + separator + signed_str + radix_str + value;
}

std::string Identifier::toString() {
  static std::unordered_set<std::string> sKeywords{
      // clang-format off
      "accept_on",     "dist",          "local",                "randomize",       "task",
      "alias",         "do",            "localparam",           "randsequence",    "this",
      "always",        "edge",          "logic",                "rcmos",           "time",
      "always_comb",   "else",          "longint",              "real",            "timeprecision",
      "always_ff",     "end",           "macromodule",          "realtime",        "timeunit",
      "always_latch",  "enum",          "matches",              "ref",             "tran",
      "and",           "event",         "modport",              "reg",             "tranif0",
      "assert",        "eventually",    "module",               "reject_on",       "tranif1",
      "assign",        "expect",        "nand",                 "release",         "tri",
      "assume",        "export",        "negedge",              "repeat",          "tri0",
      "automatic",     "extends",       "nettype",              "restrict",        "tri1",
      "begin",         "extern",        "new",                  "return",          "triand",
      "bind",          "final",         "nexttime",             "rnmos",           "trior",
      "bins",          "first_match",   "nmos",                 "rpmos",           "trireg",
      "binsof",        "for",           "nor",                  "rtran",           "type",
      "bit",           "force",         "noshowcancelled",      "rtranif0",        "type_option",
      "break",         "foreach",       "not",                  "rtranif1",        "typedef",
      "buf",           "forever",       "notif0",               "s_always",        "union",
      "bufif0",        "fork",          "notif1",               "s_eventually",    "unique",
      "bufif1",        "function",      "null",                 "s_nexttime",      "unique0",
      "byte",          "generate",      "option",               "scalared",        "unsigned",
      "case",          "genvar",        "or",                   "sequence",        "untyped",
      "casex",         "global",        "output",               "shortint",        "use",
      "casez",         "if",            "package",              "shortreal",       "uwire",
      "cell",          "iff",           "packed",               "showcancelled",   "var",
      "chandle",       "ifnone",        "parameter",            "signed",          "vectored",
      "checker",       "ignore_bins",   "pmos",                 "soft",            "virtual",
      "class",         "illegal_bins",  "posedge",              "solve",           "void",
      "clocking",      "implements",    "primitive",            "specify",         "wait",
      "cmos",          "import",        "priority",             "specparam",       "wait_order",
      "config",        "initial",       "program",              "static",          "wand",
      "const",         "inout",         "property",             "std",             "weak",
      "constraint",    "input",         "property_expr",        "string",          "weak0",
      "context",       "instance",      "protected",            "strong",          "weak1",
      "continue",      "int",           "pull0",                "strong0",         "while",
      "cover",         "integer",       "pull1",                "strong1",         "wildcard",
      "covergroup",    "interconnect",  "pulldown",             "struct",          "wire",
      "coverpoint",    "interface",     "pullup",               "super",           "with",
      "cross",         "intersect",     "pulsestyle_ondetect",  "supply0",         "wor",
      "deassign",      "join",          "pulsestyle_onevent",   "supply1",         "xnor",
      "default",       "join_any",      "pure",                 "sync_accept_on",  "xor",
      "defparam",      "join_none",     "rand",                 "sync_reject_on",
      "design",        "let",           "randc",                "table",
      "disable",       "liblist",       "randcase",             "tagged",
      // clang-format on
  };
  static std::regex sSimpleIdentifierRE{"^[a-zA-Z$_][a-zA-Z$_0-9]*$"};
  if (sKeywords.count(value) || !std::regex_match(value, sSimpleIdentifierRE))
    return "\\" + value + " ";
  else
    return value;
}

std::string String::toString() { return "\"" + value + "\""; }

std::string Index::toString() {
  return id->toString() + '[' + index->toString() + ']';
}

std::string Slice::toString() {
  return id->toString() + '[' + high_index->toString() + ':' +
         low_index->toString() + ']';
}

std::string Vector::toString() {
  return "[" + msb->toString() + ':' + lsb->toString() + "] " + id->toString();
}

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
      op_str = "&";
      break;
    case BinOp::LAND:
      op_str = "&&";
      break;
    case BinOp::OR:
      op_str = "|";
      break;
    case BinOp::LOR:
      op_str = "||";
      break;
    case BinOp::XOR:
      op_str = "^";
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
    case BinOp::LT:
      op_str = "<";
      break;
    case BinOp::LTE:
      op_str = "<=";
      break;
    case BinOp::GT:
      op_str = ">";
      break;
    case BinOp::GTE:
      op_str = ">=";
      break;
  }
  std::string lstr = left->toString();
  std::string rstr = right->toString();
  // TODO Precedence logic, for now we just insert parens if not symbol or num
  if (auto ptr = dynamic_cast<Identifier*>(left.get())) {
  } else if (auto ptr = dynamic_cast<NumericLiteral*>(left.get())) {
  } else {
      lstr = "(" + lstr + ")";
  }
  if (auto ptr = dynamic_cast<Identifier*>(right.get())) {
  } else if (auto ptr = dynamic_cast<NumericLiteral*>(right.get())) {
  } else {
      rstr = "(" + rstr + ")";
  }
  return lstr + ' ' + op_str + ' ' + rstr;
}

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
}

std::string TernaryOp::toString() {
  return cond->toString() + " ? " + true_value->toString() + " : " +
         false_value->toString();
}

std::string Concat::toString() {
  std::vector<std::string> arg_strs;
  for (auto &arg : args) {
    arg_strs.push_back(arg->toString());
  }
  return "{" + join(arg_strs, ",") + "}";
}

std::string Replicate::toString() {
  // TODO: Insert parens using precedence logic
  return "{(" + num->toString() + "){" + value->toString() + "}" + "}";
}

std::string NegEdge::toString() { return "negedge " + value->toString(); }

std::string PosEdge::toString() { return "posedge " + value->toString(); }

std::string Call::toString() {
  std::vector<std::string> arg_strs;
  for (auto &arg : args) {
    arg_strs.push_back(arg->toString());
  }
  return func + "(" + join(arg_strs, ", ") + ")";
}

template <typename... Ts>
std::string variant_to_string(std::variant<Ts...> &value) {
  return std::visit(
      [](auto &&value) -> std::string { return value->toString(); }, value);
}

std::string Port::toString() {
  std::string value_str =
      variant_to_string<std::unique_ptr<Expression>, std::unique_ptr<Vector>>(
          value);
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

std::string Module::emitModuleHeader() {
  std::string module_header_str = "module " + name;

  // emit parameter string
  if (!parameters.empty()) {
    module_header_str += " #(";
    std::vector<std::string> param_strs;
    for (auto &it : parameters) {
      param_strs.push_back("parameter " + it.first->toString() + " = " +
                           it.second->toString());
    }
    module_header_str += join(param_strs, ", ");
    module_header_str += ")";
  }

  // emit port string
  module_header_str += " (";
  std::vector<std::string> ports_strs;
  for (auto &it : ports) ports_strs.push_back(it->toString());
  module_header_str += join(ports_strs, ", ");
  module_header_str += ");\n";
  return module_header_str;
}

std::string Module::toString() {
  std::string module_str = "";
  module_str += emitModuleHeader();

  // emit body
  for (auto &statement : body) {
    module_str += variant_to_string<std::unique_ptr<StructuralStatement>,
                                    std::unique_ptr<Declaration>>(statement) +
                  "\n";
  }

  module_str += "endmodule\n";
  return module_str;
}

std::string StringBodyModule::toString() {
  std::string module_str = "";
  module_str += emitModuleHeader();
  module_str += body;
  module_str += "\nendmodule\n";
  return module_str;
}

std::string ModuleInstantiation::toString() {
  std::string module_inst_str = "";
  module_inst_str += module_name;
  if (!parameters.empty()) {
    module_inst_str += " #(";
    std::vector<std::string> param_strs;
    for (auto &it : parameters) {
      param_strs.push_back("." + it.first->toString() + "(" +
                           it.second->toString() + ")");
    }
    module_inst_str += join(param_strs, ", ");
    module_inst_str += ")";
  }
  module_inst_str += " " + instance_name + "(";
  if (!connections.empty()) {
    std::vector<std::string> param_strs;
    for (auto &it : connections) {
      param_strs.push_back("." + it.first + "(" + it.second->toString() + ")");
    }
    module_inst_str += join(param_strs, ", ");
  }
  module_inst_str += ");";
  return module_inst_str;
}

std::string Declaration::toString() {
  return decl + " " + variant_to_string(value) + ";";
}

std::string Assign::toString() {
  return prefix + variant_to_string(target) + " " + symbol + " " +
         value->toString() + ";";
}

std::string Always::toString() {
  std::string always_str = "";
  always_str += "always @(";

  // emit sensitivity string
  std::vector<std::string> sensitivity_strs;
  for (auto &it : sensitivity_list) {
    sensitivity_strs.push_back(it->toString());
  }
  always_str += join(sensitivity_strs, ", ");
  always_str += ") begin\n";

  // emit body
  for (auto &statement : body) {
    always_str += variant_to_string<std::unique_ptr<BehavioralStatement>,
                                    std::unique_ptr<Declaration>>(statement) +
                  "\n";
  }

  always_str += "end\n";
  return always_str;
}

std::string File::toString() {
  std::string file_str = "";

  std::vector<std::string> file_strs;
  for (auto &module : modules) {
    file_strs.push_back(module->toString());
  }

  return join(file_strs, "\n");
}

std::unique_ptr<Identifier> make_id(std::string name) {
  return std::make_unique<Identifier>(name);
}

std::unique_ptr<NumericLiteral> make_num(std::string val) {
  return std::make_unique<NumericLiteral>(val);
}

std::unique_ptr<BinaryOp> make_binop(std::unique_ptr<Expression> left,
                                     BinOp::BinOp op,
                                     std::unique_ptr<Expression> right) {
  return std::make_unique<BinaryOp>(std::move(left), op, std::move(right));
}

std::unique_ptr<Port> make_port(
    std::variant<std::unique_ptr<Expression>, std::unique_ptr<Vector>> value,
    Direction direction, PortType data_type) {
  return std::make_unique<Port>(std::move(value), direction, data_type);
}

std::unique_ptr<Vector> make_vector(std::unique_ptr<Expression> id,
                                    std::unique_ptr<Expression> msb,
                                    std::unique_ptr<Expression> lsb) {
  return std::make_unique<Vector>(std::move(id), std::move(msb),
                                  std::move(lsb));
}

}  // namespace verilogAST
