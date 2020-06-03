#include "verilogAST.hpp"

namespace vAST = verilogAST;

vAST::Parameters make_simple_params() {
  vAST::Parameters parameters;
  parameters.push_back(
      std::make_pair(vAST::make_id("param0"), vAST::make_num("0")));
  parameters.push_back(
      std::make_pair(vAST::make_id("param1"), vAST::make_num("1")));
  return parameters;
}

std::unique_ptr<vAST::Connections> make_simple_connections() {
  std::unique_ptr<vAST::Connections> connections =
      std::make_unique<vAST::Connections>();
  connections->insert("a", vAST::make_id("a"));
  connections->insert("b", std::make_unique<vAST::Index>(vAST::make_id("b"),
                                                         vAST::make_num("0")));
  connections->insert(
      "c", std::make_unique<vAST::Slice>(
               vAST::make_id("c"), vAST::make_num("31"), vAST::make_num("0")));

  return connections;
}

std::vector<std::unique_ptr<vAST::AbstractPort>> make_simple_ports() {
  std::vector<std::unique_ptr<vAST::AbstractPort>> ports;
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("i"), vAST::INPUT,
                                               vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("o"), vAST::OUTPUT,
                                               vAST::WIRE));
  return ports;
}

std::vector<std::variant<std::unique_ptr<vAST::StructuralStatement>,
                         std::unique_ptr<vAST::Declaration>>>
make_simple_body() {
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

std::vector<std::variant<std::unique_ptr<vAST::BehavioralStatement>,
                         std::unique_ptr<vAST::Declaration>>>
make_simple_always_body() {
  std::vector<std::variant<std::unique_ptr<vAST::BehavioralStatement>,
                           std::unique_ptr<vAST::Declaration>>>
      body;
  body.push_back(std::make_unique<vAST::BlockingAssign>(
      std::make_unique<vAST::Identifier>("a"),
      std::make_unique<vAST::Identifier>("b")));
  body.push_back(std::make_unique<vAST::NonBlockingAssign>(
      std::make_unique<vAST::Identifier>("b"),
      std::make_unique<vAST::Identifier>("c")));
  std::vector<std::unique_ptr<vAST::Expression>> args;
  args.push_back(std::make_unique<vAST::String>("b=%d, c=%d"));
  args.push_back(std::make_unique<vAST::Identifier>("b"));
  args.push_back(std::make_unique<vAST::Identifier>("c"));
  body.push_back(std::make_unique<vAST::CallStmt>("$display", std::move(args)));

  std::vector<std::unique_ptr<vAST::BehavioralStatement>> true_body;
  true_body.push_back(std::make_unique<vAST::BlockingAssign>(
      std::make_unique<vAST::Identifier>("e"),
      std::make_unique<vAST::Identifier>("f")));

  std::vector<std::unique_ptr<vAST::BehavioralStatement>> false_body;
  false_body.push_back(std::make_unique<vAST::BlockingAssign>(
      std::make_unique<vAST::Identifier>("e"),
      std::make_unique<vAST::Identifier>("g")));

  body.push_back(std::make_unique<vAST::If>(
      vAST::make_id("b"), std::move(true_body), std::move(false_body)));

  return body;
}
