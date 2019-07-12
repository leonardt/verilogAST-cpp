#include "verilogAST.hpp"

namespace vAST = verilogAST;

std::unique_ptr<vAST::Identifier> make_id(std::string name) {
  return std::make_unique<vAST::Identifier>(name);
}

std::unique_ptr<vAST::NumericLiteral> make_num(std::string val) {
  return std::make_unique<vAST::NumericLiteral>(val);
}

std::unique_ptr<vAST::BinaryOp> make_binop(
    std::unique_ptr<vAST::Expression> left, vAST::BinOp::BinOp op,
    std::unique_ptr<vAST::Expression> right) {
  return std::make_unique<vAST::BinaryOp>(std::move(left), op,
                                          std::move(right));
}

std::unique_ptr<vAST::Port> make_port(
    std::variant<std::unique_ptr<vAST::Identifier>,
                 std::unique_ptr<vAST::Vector>>
        value,
    vAST::Direction direction, vAST::PortType data_type) {
  return std::make_unique<vAST::Port>(std::move(value), direction, data_type);
}

std::unique_ptr<vAST::Vector> make_vector(
    std::unique_ptr<vAST::Identifier> id, std::unique_ptr<vAST::Expression> msb,
    std::unique_ptr<vAST::Expression> lsb) {
  return std::make_unique<vAST::Vector>(std::move(id), std::move(msb),
                                        std::move(lsb));
}

vAST::Parameters make_simple_params() {
  vAST::Parameters parameters;
  parameters.push_back(std::make_pair(make_id("param0"), make_num("0")));
  parameters.push_back(std::make_pair(make_id("param1"), make_num("1")));
  return parameters;
}

std::map<std::string, std::variant<std::unique_ptr<vAST::Identifier>,
                                   std::unique_ptr<vAST::Index>,
                                   std::unique_ptr<vAST::Slice>>>
make_simple_connections() {
  std::map<std::string, std::variant<std::unique_ptr<vAST::Identifier>,
                                     std::unique_ptr<vAST::Index>,
                                     std::unique_ptr<vAST::Slice>>>
      connections;
  connections["a"] = make_id("a");
  connections["b"] = std::make_unique<vAST::Index>(make_id("b"), make_num("0"));
  connections["c"] = std::make_unique<vAST::Slice>(make_id("c"), make_num("31"),
                                                   make_num("0"));

  return connections;
}

std::vector<std::unique_ptr<vAST::AbstractPort>> make_simple_ports() {
  std::vector<std::unique_ptr<vAST::AbstractPort>> ports;
  ports.push_back(
      std::make_unique<vAST::Port>(make_id("i"), vAST::INPUT, vAST::WIRE));
  ports.push_back(
      std::make_unique<vAST::Port>(make_id("o"), vAST::OUTPUT, vAST::WIRE));
  return ports;
}

std::vector<std::variant<std::unique_ptr<vAST::StructuralStatement>,
    std::unique_ptr<vAST::Declaration>>> make_simple_body() {
  std::vector<std::variant<std::unique_ptr<vAST::StructuralStatement>,
                           std::unique_ptr<vAST::Declaration>>>
      body;

  std::string module_name = "other_module";
  std::string instance_name = "other_module_inst";

  body.push_back(std::make_unique<vAST::ModuleInstantiation>(
      module_name, make_simple_params(), instance_name,
      make_simple_connections()));
  return body;
}
