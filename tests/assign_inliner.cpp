#include "verilogAST/assign_inliner.hpp"
#include "common.cpp"
#include "gtest/gtest.h"

namespace vAST = verilogAST;

class AssignInliner : public vAST::Transformer {
 public:
  using vAST::Transformer::visit;
};

namespace {
TEST(InlineAssignTests, TestBasic) {
  std::vector<std::unique_ptr<vAST::AbstractPort>> ports;
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("i"), vAST::INPUT,
                                               vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("o"), vAST::OUTPUT,
                                               vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(
      std::make_unique<vAST::Vector>(vAST::make_id("o_vec"),
                                     vAST::make_num("1"), vAST::make_num("0")),
      vAST::OUTPUT, vAST::WIRE));

  std::vector<std::variant<std::unique_ptr<vAST::StructuralStatement>,
                           std::unique_ptr<vAST::Declaration>>>
      body;

  body.push_back(
      std::make_unique<vAST::Wire>(std::make_unique<vAST::Identifier>("x")));

  body.push_back(std::make_unique<vAST::Wire>(std::make_unique<vAST::Vector>(
      vAST::make_id("x_vec"), vAST::make_num("1"), vAST::make_num("0"))));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("x"),
      std::make_unique<vAST::Identifier>("i")));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("o"),
      std::make_unique<vAST::Identifier>("x")));

  std::vector<std::unique_ptr<vAST::Expression>> concat_args;
  concat_args.push_back(std::make_unique<vAST::Identifier>("i"));
  concat_args.push_back(std::make_unique<vAST::Identifier>("i"));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("x_vec"),
      std::make_unique<vAST::Concat>(std::move(concat_args))));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("o_vec"),
      std::make_unique<vAST::Identifier>("x_vec")));

  std::unique_ptr<vAST::AbstractModule> module = std::make_unique<vAST::Module>(
      "test_module", std::move(ports), std::move(body));

  std::string raw_str =
      "module test_module (\n"
      "    input i,\n"
      "    output o,\n"
      "    output [1:0] o_vec\n"
      ");\n"
      "wire x;\n"
      "wire [1:0] x_vec;\n"
      "assign x = i;\n"
      "assign o = x;\n"
      "assign x_vec = {i,i};\n"
      "assign o_vec = x_vec;\n"
      "endmodule\n";

  EXPECT_EQ(module->toString(), raw_str);

  std::set<std::string> blacklist = {"x", "x_vec"};
  vAST::AssignInliner transformer_blacklist(blacklist);
  module = transformer_blacklist.visit(std::move(module));
  EXPECT_EQ(module->toString(), raw_str);

  std::string expected_str =
      "module test_module (\n"
      "    input i,\n"
      "    output o,\n"
      "    output [1:0] o_vec\n"
      ");\n"
      "wire [1:0] x_vec;\n"
      "assign o = i;\n"
      "assign x_vec = {i,i};\n"
      "assign o_vec = x_vec;\n"
      "endmodule\n";

  blacklist = {"x_vec"};
  vAST::AssignInliner transformer_blacklist2(blacklist);
  module = transformer_blacklist2.visit(std::move(module));
  EXPECT_EQ(module->toString(), expected_str);

  expected_str =
      "module test_module (\n"
      "    input i,\n"
      "    output o,\n"
      "    output [1:0] o_vec\n"
      ");\n"
      "assign o = i;\n"
      "assign o_vec = {i,i};\n"
      "endmodule\n";

  vAST::AssignInliner transformer;
  EXPECT_EQ(transformer.visit(std::move(module))->toString(), expected_str);
}

TEST(InlineAssignTests, TestBasicChain) {
  std::vector<std::unique_ptr<vAST::AbstractPort>> ports;
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("i"), vAST::INPUT,
                                               vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("o"), vAST::OUTPUT,
                                               vAST::WIRE));

  std::vector<std::variant<std::unique_ptr<vAST::StructuralStatement>,
                           std::unique_ptr<vAST::Declaration>>>
      body;

  body.push_back(
      std::make_unique<vAST::Wire>(std::make_unique<vAST::Identifier>("x")));

  body.push_back(
      std::make_unique<vAST::Wire>(std::make_unique<vAST::Identifier>("y")));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("x"),
      std::make_unique<vAST::Identifier>("i")));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("y"),
      std::make_unique<vAST::Identifier>("x")));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("o"),
      std::make_unique<vAST::Identifier>("y")));

  std::unique_ptr<vAST::AbstractModule> module = std::make_unique<vAST::Module>(
      "test_module", std::move(ports), std::move(body));

  std::string raw_str =
      "module test_module (\n"
      "    input i,\n"
      "    output o\n"
      ");\n"
      "wire x;\n"
      "wire y;\n"
      "assign x = i;\n"
      "assign y = x;\n"
      "assign o = y;\n"
      "endmodule\n";

  EXPECT_EQ(module->toString(), raw_str);

  std::string expected_str =
      "module test_module (\n"
      "    input i,\n"
      "    output o\n"
      ");\n"
      "assign o = i;\n"
      "endmodule\n";

  vAST::AssignInliner transformer;
  EXPECT_EQ(transformer.visit(std::move(module))->toString(), expected_str);
}

TEST(InlineAssignTests, TestBasicExpr) {
  std::vector<std::unique_ptr<vAST::AbstractPort>> ports;
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("i"), vAST::INPUT,
                                               vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("o"), vAST::OUTPUT,
                                               vAST::WIRE));

  std::vector<std::variant<std::unique_ptr<vAST::StructuralStatement>,
                           std::unique_ptr<vAST::Declaration>>>
      body;

  body.push_back(
      std::make_unique<vAST::Wire>(std::make_unique<vAST::Identifier>("x")));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("x"),
      vAST::make_binop(std::make_unique<vAST::Identifier>("i"),
                       vAST::BinOp::XOR,
                       std::make_unique<vAST::NumericLiteral>("1"))));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("o"),
      std::make_unique<vAST::Identifier>("x")));

  std::unique_ptr<vAST::AbstractModule> module = std::make_unique<vAST::Module>(
      "test_module", std::move(ports), std::move(body));

  std::string raw_str =
      "module test_module (\n"
      "    input i,\n"
      "    output o\n"
      ");\n"
      "wire x;\n"
      "assign x = i ^ 1;\n"
      "assign o = x;\n"
      "endmodule\n";

  EXPECT_EQ(module->toString(), raw_str);

  std::string expected_str =
      "module test_module (\n"
      "    input i,\n"
      "    output o\n"
      ");\n"
      "assign o = i ^ 1;\n"
      "endmodule\n";

  vAST::AssignInliner transformer;
  EXPECT_EQ(transformer.visit(std::move(module))->toString(), expected_str);
}

TEST(InlineAssignTests, TestExprChain) {
  std::vector<std::unique_ptr<vAST::AbstractPort>> ports;
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("i"), vAST::INPUT,
                                               vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("o"), vAST::OUTPUT,
                                               vAST::WIRE));

  std::vector<std::variant<std::unique_ptr<vAST::StructuralStatement>,
                           std::unique_ptr<vAST::Declaration>>>
      body;

  body.push_back(
      std::make_unique<vAST::Wire>(std::make_unique<vAST::Identifier>("x")));

  body.push_back(
      std::make_unique<vAST::Wire>(std::make_unique<vAST::Identifier>("y")));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("x"),
      vAST::make_binop(std::make_unique<vAST::Identifier>("i"),
                       vAST::BinOp::XOR,
                       std::make_unique<vAST::NumericLiteral>("1"))));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("y"),
      std::make_unique<vAST::Identifier>("x")));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("o"),
      std::make_unique<vAST::Identifier>("y")));

  std::unique_ptr<vAST::AbstractModule> module = std::make_unique<vAST::Module>(
      "test_module", std::move(ports), std::move(body));

  std::string raw_str =
      "module test_module (\n"
      "    input i,\n"
      "    output o\n"
      ");\n"
      "wire x;\n"
      "wire y;\n"
      "assign x = i ^ 1;\n"
      "assign y = x;\n"
      "assign o = y;\n"
      "endmodule\n";

  EXPECT_EQ(module->toString(), raw_str);

  std::string expected_str =
      "module test_module (\n"
      "    input i,\n"
      "    output o\n"
      ");\n"
      "assign o = i ^ 1;\n"
      "endmodule\n";

  vAST::AssignInliner transformer;
  EXPECT_EQ(transformer.visit(std::move(module))->toString(), expected_str);
}

TEST(InlineAssignTests, TestInlineFanOutIdOrNum) {
  std::vector<std::unique_ptr<vAST::AbstractPort>> ports;
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("i"), vAST::INPUT,
                                               vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(
      std::make_unique<vAST::Vector>(vAST::make_id("i_vec"),
                                     vAST::make_num("1"), vAST::make_num("0")),
      vAST::INPUT, vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("o0"),
                                               vAST::OUTPUT, vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("o1"),
                                               vAST::OUTPUT, vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("o2"),
                                               vAST::OUTPUT, vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("o3"),
                                               vAST::OUTPUT, vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(
      std::make_unique<vAST::Vector>(vAST::make_id("o_vec0"),
                                     vAST::make_num("1"), vAST::make_num("0")),
      vAST::OUTPUT, vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(
      std::make_unique<vAST::Vector>(vAST::make_id("o_vec1"),
                                     vAST::make_num("1"), vAST::make_num("0")),
      vAST::OUTPUT, vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(
      std::make_unique<vAST::Vector>(vAST::make_id("o_vec2"),
                                     vAST::make_num("1"), vAST::make_num("0")),
      vAST::OUTPUT, vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(
      std::make_unique<vAST::Vector>(vAST::make_id("o_vec3"),
                                     vAST::make_num("1"), vAST::make_num("0")),
      vAST::OUTPUT, vAST::WIRE));

  std::vector<std::variant<std::unique_ptr<vAST::StructuralStatement>,
                           std::unique_ptr<vAST::Declaration>>>
      body;

  body.push_back(
      std::make_unique<vAST::Wire>(std::make_unique<vAST::Identifier>("x")));

  body.push_back(
      std::make_unique<vAST::Wire>(std::make_unique<vAST::Identifier>("y")));

  body.push_back(std::make_unique<vAST::Wire>(std::make_unique<vAST::Vector>(
      vAST::make_id("x_vec"), vAST::make_num("1"), vAST::make_num("0"))));

  body.push_back(std::make_unique<vAST::Wire>(std::make_unique<vAST::Vector>(
      vAST::make_id("y_vec"), vAST::make_num("1"), vAST::make_num("0"))));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("x"),
      std::make_unique<vAST::Identifier>("i")));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("o0"),
      std::make_unique<vAST::Identifier>("x")));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("o1"),
      std::make_unique<vAST::Identifier>("x")));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("y"),
      std::make_unique<vAST::NumericLiteral>("1")));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("o2"),
      std::make_unique<vAST::Identifier>("y")));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("o3"),
      std::make_unique<vAST::Identifier>("y")));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("x_vec"),
      std::make_unique<vAST::Identifier>("i_vec")));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("y_vec"),
      std::make_unique<vAST::NumericLiteral>("2'b10")));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("o_vec0"),
      std::make_unique<vAST::Identifier>("x_vec")));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("o_vec1"),
      std::make_unique<vAST::Identifier>("x_vec")));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("o_vec2"),
      std::make_unique<vAST::Identifier>("y_vec")));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("o_vec3"),
      std::make_unique<vAST::Identifier>("y_vec")));

  std::unique_ptr<vAST::AbstractModule> module = std::make_unique<vAST::Module>(
      "test_module", std::move(ports), std::move(body));

  std::string raw_str =
      "module test_module (\n"
      "    input i,\n"
      "    input [1:0] i_vec,\n"
      "    output o0,\n"
      "    output o1,\n"
      "    output o2,\n"
      "    output o3,\n"
      "    output [1:0] o_vec0,\n"
      "    output [1:0] o_vec1,\n"
      "    output [1:0] o_vec2,\n"
      "    output [1:0] o_vec3\n"
      ");\n"
      "wire x;\n"
      "wire y;\n"
      "wire [1:0] x_vec;\n"
      "wire [1:0] y_vec;\n"
      "assign x = i;\n"
      "assign o0 = x;\n"
      "assign o1 = x;\n"
      "assign y = 1;\n"
      "assign o2 = y;\n"
      "assign o3 = y;\n"
      "assign x_vec = i_vec;\n"
      "assign y_vec = 2'b10;\n"
      "assign o_vec0 = x_vec;\n"
      "assign o_vec1 = x_vec;\n"
      "assign o_vec2 = y_vec;\n"
      "assign o_vec3 = y_vec;\n"
      "endmodule\n";

  EXPECT_EQ(module->toString(), raw_str);

  std::string expected_str =
      "module test_module (\n"
      "    input i,\n"
      "    input [1:0] i_vec,\n"
      "    output o0,\n"
      "    output o1,\n"
      "    output o2,\n"
      "    output o3,\n"
      "    output [1:0] o_vec0,\n"
      "    output [1:0] o_vec1,\n"
      "    output [1:0] o_vec2,\n"
      "    output [1:0] o_vec3\n"
      ");\n"
      "assign o0 = i;\n"
      "assign o1 = i;\n"
      "assign o2 = 1;\n"
      "assign o3 = 1;\n"
      "assign o_vec0 = i_vec;\n"
      "assign o_vec1 = i_vec;\n"
      "assign o_vec2 = 2'b10;\n"
      "assign o_vec3 = 2'b10;\n"
      "endmodule\n";
  vAST::AssignInliner transformer;
  EXPECT_EQ(transformer.visit(std::move(module))->toString(), expected_str);
}

TEST(InlineAssignTests, TestNoInlineFanOutExpr) {
  std::vector<std::unique_ptr<vAST::AbstractPort>> ports;
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("i"), vAST::INPUT,
                                               vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("o0"),
                                               vAST::OUTPUT, vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("o1"),
                                               vAST::OUTPUT, vAST::WIRE));

  std::vector<std::variant<std::unique_ptr<vAST::StructuralStatement>,
                           std::unique_ptr<vAST::Declaration>>>
      body;

  body.push_back(
      std::make_unique<vAST::Wire>(std::make_unique<vAST::Identifier>("x")));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("x"),
      vAST::make_binop(std::make_unique<vAST::Identifier>("i"),
                       vAST::BinOp::XOR,
                       std::make_unique<vAST::NumericLiteral>("1"))));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("o0"),
      std::make_unique<vAST::Identifier>("x")));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("o1"),
      std::make_unique<vAST::Identifier>("x")));

  std::unique_ptr<vAST::AbstractModule> module = std::make_unique<vAST::Module>(
      "test_module", std::move(ports), std::move(body));

  std::string raw_str =
      "module test_module (\n"
      "    input i,\n"
      "    output o0,\n"
      "    output o1\n"
      ");\n"
      "wire x;\n"
      "assign x = i ^ 1;\n"
      "assign o0 = x;\n"
      "assign o1 = x;\n"
      "endmodule\n";

  EXPECT_EQ(module->toString(), raw_str);

  std::string expected_str = raw_str;
  vAST::AssignInliner transformer;
  EXPECT_EQ(transformer.visit(std::move(module))->toString(), expected_str);
}

TEST(InlineAssignTests, TestMultipleAssign) {
  // Should not inline a wire that is assigned multiple times
  std::vector<std::unique_ptr<vAST::AbstractPort>> ports;
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("io0"),
                                               vAST::INOUT, vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("io1"),
                                               vAST::INOUT, vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("io2"),
                                               vAST::INOUT, vAST::WIRE));

  std::vector<std::variant<std::unique_ptr<vAST::StructuralStatement>,
                           std::unique_ptr<vAST::Declaration>>>
      body;

  body.push_back(
      std::make_unique<vAST::Wire>(std::make_unique<vAST::Identifier>("x")));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("x"),
      std::make_unique<vAST::Identifier>("io0")));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("x"),
      std::make_unique<vAST::Identifier>("io1")));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("io2"),
      std::make_unique<vAST::Identifier>("x")));

  std::unique_ptr<vAST::AbstractModule> module = std::make_unique<vAST::Module>(
      "test_module", std::move(ports), std::move(body));

  std::string raw_str =
      "module test_module (\n"
      "    inout io0,\n"
      "    inout io1,\n"
      "    inout io2\n"
      ");\n"
      "wire x;\n"
      "assign x = io0;\n"
      "assign x = io1;\n"
      "assign io2 = x;\n"
      "endmodule\n";

  EXPECT_EQ(module->toString(), raw_str);

  std::string expected_str = raw_str;
  vAST::AssignInliner transformer;
  EXPECT_EQ(transformer.visit(std::move(module))->toString(), expected_str);
}

TEST(InlineAssignTests, TestInstConn) {
  // Should not inline a wire that is assigned multiple times
  std::vector<std::unique_ptr<vAST::AbstractPort>> ports;
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("i"),
                                               vAST::INPUT, vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("a"),
                                               vAST::INPUT, vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("o"),
                                               vAST::OUTPUT, vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("b"),
                                               vAST::OUTPUT, vAST::WIRE));

  std::vector<std::variant<std::unique_ptr<vAST::StructuralStatement>,
                           std::unique_ptr<vAST::Declaration>>>
      body;

  body.push_back(
      std::make_unique<vAST::Wire>(std::make_unique<vAST::Identifier>("x")));

  body.push_back(
      std::make_unique<vAST::Wire>(std::make_unique<vAST::Identifier>("y")));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("x"),
      std::make_unique<vAST::Identifier>("i")));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("b"),
      std::make_unique<vAST::Identifier>("a")));

  vAST::Parameters parameters;
  std::unique_ptr<vAST::Connections> connections = std::make_unique<vAST::Connections>();
  connections->insert("c", vAST::make_id("a"));
  connections->insert("i", vAST::make_id("x"));
  connections->insert("o", vAST::make_id("y"));

  body.push_back(std::make_unique<vAST::ModuleInstantiation>(
      "inner_module", std::move(parameters), "inner_module_inst",
      std::move(connections)));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("o"),
      std::make_unique<vAST::Identifier>("y")));

  std::unique_ptr<vAST::AbstractModule> module = std::make_unique<vAST::Module>(
      "test_module", std::move(ports), std::move(body));

  std::string raw_str =
      "module test_module (\n"
      "    input i,\n"
      "    input a,\n"
      "    output o,\n"
      "    output b\n"
      ");\n"
      "wire x;\n"
      "wire y;\n"
      "assign x = i;\n"
      "assign b = a;\n"
      "inner_module inner_module_inst (\n"
      "    .c(a),\n"
      "    .i(x),\n"
      "    .o(y)\n"
      ");\n"
      "assign o = y;\n"
      "endmodule\n";

  EXPECT_EQ(module->toString(), raw_str);

  std::string expected_str =
      "module test_module (\n"
      "    input i,\n"
      "    input a,\n"
      "    output o,\n"
      "    output b\n"
      ");\n"
      "assign b = a;\n"
      "inner_module inner_module_inst (\n"
      "    .c(a),\n"
      "    .i(i),\n"
      "    .o(o)\n"
      ");\n"
      "endmodule\n";
  vAST::AssignInliner transformer;
  EXPECT_EQ(transformer.visit(std::move(module))->toString(), expected_str);
}

TEST(InlineAssignTests, TestNoInlineIndex) {
  std::vector<std::unique_ptr<vAST::AbstractPort>> ports;
  ports.push_back(std::make_unique<vAST::Port>(
      std::make_unique<vAST::Vector>(vAST::make_id("i1"),
                                     vAST::make_num("1"), vAST::make_num("0")),
      vAST::INPUT, vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(
      std::make_unique<vAST::Vector>(vAST::make_id("i2"),
                                     vAST::make_num("1"), vAST::make_num("0")),
      vAST::INPUT, vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("o"), vAST::OUTPUT,
                                               vAST::WIRE));

  std::vector<std::variant<std::unique_ptr<vAST::StructuralStatement>,
                           std::unique_ptr<vAST::Declaration>>>
      body;

  body.push_back(
      std::make_unique<vAST::Wire>(std::make_unique<vAST::Identifier>("x")));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      vAST::make_id("x"),
      std::make_unique<vAST::BinaryOp>(vAST::make_id("i1"), vAST::BinOp::ADD,
                                       vAST::make_id("i2"))));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      vAST::make_id("o"),
      std::make_unique<vAST::Index>(vAST::make_id("x"), vAST::make_num("0"))));

  std::unique_ptr<vAST::AbstractModule> module = std::make_unique<vAST::Module>(
      "test_module", std::move(ports), std::move(body));

  std::string raw_str =
      "module test_module (\n"
      "    input [1:0] i1,\n"
      "    input [1:0] i2,\n"
      "    output o\n"
      ");\n"
      "wire x;\n"
      "assign x = i1 + i2;\n"
      "assign o = x[0];\n"
      "endmodule\n";

  EXPECT_EQ(module->toString(), raw_str);

  vAST::AssignInliner transformer;
  EXPECT_EQ(transformer.visit(std::move(module))->toString(), raw_str);
}

}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
