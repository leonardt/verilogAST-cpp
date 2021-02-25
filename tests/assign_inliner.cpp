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
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("o1"),
                                               vAST::OUTPUT, vAST::WIRE));
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

  std::vector<std::unique_ptr<vAST::BehavioralStatement>> always_body;

  std::vector<std::unique_ptr<vAST::BehavioralStatement>> true_body;
  true_body.push_back(std::make_unique<vAST::BlockingAssign>(
      std::make_unique<vAST::Identifier>("o1"),
      std::make_unique<vAST::Identifier>("x")));

  std::vector<
      std::pair<std::unique_ptr<vAST::Expression>,
                std::vector<std::unique_ptr<vAST::BehavioralStatement>>>>
      else_ifs;
  for (int i = 1; i < 3; i++) {
    std::unique_ptr<vAST::Expression> cond = std::make_unique<vAST::BinaryOp>(
        std::make_unique<vAST::Identifier>("x"), vAST::BinOp::EQ,
        vAST::make_num(std::to_string(i)));
    std::vector<std::unique_ptr<vAST::BehavioralStatement>> body;
    body.push_back(std::make_unique<vAST::BlockingAssign>(
        std::make_unique<vAST::Identifier>("o1"),
        std::make_unique<vAST::BinaryOp>(
            std::make_unique<vAST::Identifier>("x"), vAST::BinOp::ADD,
            vAST::make_num(std::to_string(i)))));
    else_ifs.push_back({std::move(cond), std::move(body)});
  }

  std::vector<std::unique_ptr<vAST::BehavioralStatement>> else_body;
  else_body.push_back(std::make_unique<vAST::BlockingAssign>(
      std::make_unique<vAST::Identifier>("o1"),
      std::make_unique<vAST::BinaryOp>(std::make_unique<vAST::Identifier>("x"),
                                       vAST::BinOp::ADD, vAST::make_num("3"))));

  always_body.push_back(std::make_unique<vAST::If>(
      std::make_unique<vAST::BinaryOp>(std::make_unique<vAST::Identifier>("x"),
                                       vAST::BinOp::EQ, vAST::make_num("0")),
      std::move(true_body), std::move(else_ifs), std::move(else_body)));

  std::vector<std::variant<
      std::unique_ptr<vAST::Identifier>, std::unique_ptr<vAST::PosEdge>,
      std::unique_ptr<vAST::NegEdge>, std::unique_ptr<vAST::Star>>>
      sensitivity_list;
  sensitivity_list.push_back(std::make_unique<vAST::Star>());

  body.push_back(std::make_unique<vAST::Always>(std::move(sensitivity_list),
                                                std::move(always_body)));

  std::unique_ptr<vAST::AbstractModule> module = std::make_unique<vAST::Module>(
      "test_module", std::move(ports), std::move(body));

  std::string raw_str =
      "module test_module (\n"
      "    input i,\n"
      "    output o,\n"
      "    output o1,\n"
      "    output [1:0] o_vec\n"
      ");\n"
      "wire x;\n"
      "wire [1:0] x_vec;\n"
      "assign x = i;\n"
      "assign o = x;\n"
      "assign x_vec = {i,i};\n"
      "assign o_vec = x_vec;\n"
      "always @(*) begin\n"
      "if (x == 0) begin\n"
      "    o1 = x;\n"
      "end else if (x == 1) begin\n"
      "    o1 = x + 1;\n"
      "end else if (x == 2) begin\n"
      "    o1 = x + 2;\n"
      "end else begin\n"
      "    o1 = x + 3;\n"
      "end\n"
      "end\n\n"
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
      "    output o1,\n"
      "    output [1:0] o_vec\n"
      ");\n"
      "wire [1:0] x_vec;\n"
      "assign o = i;\n"
      "assign x_vec = {i,i};\n"
      "assign o_vec = x_vec;\n"
      "always @(*) begin\n"
      "if (i == 0) begin\n"
      "    o1 = i;\n"
      "end else if (i == 1) begin\n"
      "    o1 = i + 1;\n"
      "end else if (i == 2) begin\n"
      "    o1 = i + 2;\n"
      "end else begin\n"
      "    o1 = i + 3;\n"
      "end\n"
      "end\n\n"
      "endmodule\n";

  blacklist = {"x_vec"};
  vAST::AssignInliner transformer_blacklist2(blacklist);
  module = transformer_blacklist2.visit(std::move(module));
  EXPECT_EQ(module->toString(), expected_str);

  expected_str =
      "module test_module (\n"
      "    input i,\n"
      "    output o,\n"
      "    output o1,\n"
      "    output [1:0] o_vec\n"
      ");\n"
      "assign o = i;\n"
      "assign o_vec = {i,i};\n"
      "always @(*) begin\n"
      "if (i == 0) begin\n"
      "    o1 = i;\n"
      "end else if (i == 1) begin\n"
      "    o1 = i + 1;\n"
      "end else if (i == 2) begin\n"
      "    o1 = i + 2;\n"
      "end else begin\n"
      "    o1 = i + 3;\n"
      "end\n"
      "end\n\n"
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
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("i"), vAST::INPUT,
                                               vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("a"), vAST::INPUT,
                                               vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("o"), vAST::OUTPUT,
                                               vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("b"), vAST::OUTPUT,
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
      std::make_unique<vAST::Identifier>("b"),
      std::make_unique<vAST::Identifier>("a")));

  vAST::Parameters parameters;
  std::unique_ptr<vAST::Connections> connections =
      std::make_unique<vAST::Connections>();
  connections->insert("c", vAST::make_id("a"));
  connections->insert("i", vAST::make_id("x"));
  connections->insert("o", vAST::make_id("y"));

  std::unique_ptr<vAST::ModuleInstantiation> module_inst =
      std::make_unique<vAST::ModuleInstantiation>(
          "inner_module", std::move(parameters), "inner_module_inst",
          std::move(connections));

  std::vector<std::unique_ptr<vAST::StructuralStatement>> if_def_body;
  if_def_body.push_back(std::move(module_inst));
  std::unique_ptr<vAST::IfDef> if_def =
      std::make_unique<vAST::IfDef>("ASSERT_ON", std::move(if_def_body));

  body.push_back(std::move(if_def));

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
      "`ifdef ASSERT_ON\n"
      "inner_module inner_module_inst (\n"
      "    .c(a),\n"
      "    .i(x),\n"
      "    .o(y)\n"
      ");\n"
      "`endif\n"
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
      "`ifdef ASSERT_ON\n"
      "inner_module inner_module_inst (\n"
      "    .c(a),\n"
      "    .i(i),\n"
      "    .o(o)\n"
      ");\n"
      "`endif\n"
      "endmodule\n";
  vAST::AssignInliner transformer;
  EXPECT_EQ(transformer.visit(std::move(module))->toString(), expected_str);
}

TEST(InlineAssignTests, TestNoInlineIndex) {
  std::vector<std::unique_ptr<vAST::AbstractPort>> ports;
  ports.push_back(std::make_unique<vAST::Port>(
      std::make_unique<vAST::Vector>(vAST::make_id("i1"), vAST::make_num("1"),
                                     vAST::make_num("0")),
      vAST::INPUT, vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(
      std::make_unique<vAST::Vector>(vAST::make_id("i2"), vAST::make_num("1"),
                                     vAST::make_num("0")),
      vAST::INPUT, vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("o0"),
                                               vAST::OUTPUT, vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("o1"),
                                               vAST::OUTPUT, vAST::WIRE));

  std::vector<std::variant<std::unique_ptr<vAST::StructuralStatement>,
                           std::unique_ptr<vAST::Declaration>>>
      body;

  body.push_back(
      std::make_unique<vAST::Wire>(std::make_unique<vAST::Identifier>("x")));

  body.push_back(
      std::make_unique<vAST::Wire>(std::make_unique<vAST::Identifier>("y")));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      vAST::make_id("x"),
      std::make_unique<vAST::BinaryOp>(vAST::make_id("i1"), vAST::BinOp::ADD,
                                       vAST::make_id("i2"))));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      vAST::make_id("o0"),
      std::make_unique<vAST::Index>(vAST::make_id("x"), vAST::make_num("0"))));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(vAST::make_id("y"),
                                                          vAST::make_id("i1")));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      vAST::make_id("o1"),
      std::make_unique<vAST::Index>(vAST::make_id("y"), vAST::make_num("0"))));

  std::unique_ptr<vAST::AbstractModule> module = std::make_unique<vAST::Module>(
      "test_module", std::move(ports), std::move(body));

  std::string raw_str =
      "module test_module (\n"
      "    input [1:0] i1,\n"
      "    input [1:0] i2,\n"
      "    output o0,\n"
      "    output o1\n"
      ");\n"
      "wire x;\n"
      "wire y;\n"
      "assign x = i1 + i2;\n"
      "assign o0 = x[0];\n"
      "assign y = i1;\n"
      "assign o1 = y[0];\n"
      "endmodule\n";

  EXPECT_EQ(module->toString(), raw_str);

  std::string expected_str =
      "module test_module (\n"
      "    input [1:0] i1,\n"
      "    input [1:0] i2,\n"
      "    output o0,\n"
      "    output o1\n"
      ");\n"
      "wire x;\n"
      "assign x = i1 + i2;\n"
      "assign o0 = x[0];\n"
      "assign o1 = i1[0];\n"
      "endmodule\n";

  vAST::AssignInliner transformer;
  EXPECT_EQ(transformer.visit(std::move(module))->toString(), expected_str);
}

TEST(InlineAssignTests, TestNoInlineSliceUnlessID) {
  std::vector<std::unique_ptr<vAST::AbstractPort>> ports;
  ports.push_back(std::make_unique<vAST::Port>(
      std::make_unique<vAST::Vector>(vAST::make_id("i1"), vAST::make_num("4"),
                                     vAST::make_num("0")),
      vAST::INPUT, vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(
      std::make_unique<vAST::Vector>(vAST::make_id("i2"), vAST::make_num("4"),
                                     vAST::make_num("0")),
      vAST::INPUT, vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(
      std::make_unique<vAST::Vector>(vAST::make_id("o0"), vAST::make_num("3"),
                                     vAST::make_num("0")),
      vAST::OUTPUT, vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(
      std::make_unique<vAST::Vector>(vAST::make_id("o1"), vAST::make_num("3"),
                                     vAST::make_num("0")),
      vAST::OUTPUT, vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(
      std::make_unique<vAST::Vector>(vAST::make_id("o2"), vAST::make_num("3"),
                                     vAST::make_num("0")),
      vAST::OUTPUT, vAST::WIRE));

  std::vector<std::variant<std::unique_ptr<vAST::StructuralStatement>,
                           std::unique_ptr<vAST::Declaration>>>
      body;

  body.push_back(std::make_unique<vAST::Wire>(std::make_unique<vAST::Vector>(
      vAST::make_id("x"), vAST::make_num("4"), vAST::make_num("0"))));

  body.push_back(std::make_unique<vAST::Wire>(std::make_unique<vAST::Vector>(
      vAST::make_id("y"), vAST::make_num("4"), vAST::make_num("0"))));

  body.push_back(std::make_unique<vAST::Wire>(std::make_unique<vAST::Vector>(
      vAST::make_id("h"), vAST::make_num("4"), vAST::make_num("0"))));

  body.push_back(std::make_unique<vAST::Wire>(std::make_unique<vAST::Vector>(
      vAST::make_id("g"), vAST::make_num("4"), vAST::make_num("0"))));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      vAST::make_id("x"),
      std::make_unique<vAST::BinaryOp>(vAST::make_id("i1"), vAST::BinOp::ADD,
                                       vAST::make_id("i2"))));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      vAST::make_id("h"),
      std::make_unique<vAST::BinaryOp>(vAST::make_id("i1"), vAST::BinOp::SUB,
                                       vAST::make_id("i2"))));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(vAST::make_id("g"),
                                                          vAST::make_id("h")));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      vAST::make_id("o0"),
      std::make_unique<vAST::Slice>(vAST::make_id("x"), vAST::make_num("3"),
                                    vAST::make_num("0"))));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(vAST::make_id("y"),
                                                          vAST::make_id("i1")));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      vAST::make_id("o1"),
      std::make_unique<vAST::Slice>(vAST::make_id("y"), vAST::make_num("3"),
                                    vAST::make_num("0"))));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      vAST::make_id("o2"),
      std::make_unique<vAST::Slice>(vAST::make_id("g"), vAST::make_num("3"),
                                    vAST::make_num("0"))));

  std::unique_ptr<vAST::AbstractModule> module = std::make_unique<vAST::Module>(
      "test_module", std::move(ports), std::move(body));

  std::string raw_str =
      "module test_module (\n"
      "    input [4:0] i1,\n"
      "    input [4:0] i2,\n"
      "    output [3:0] o0,\n"
      "    output [3:0] o1,\n"
      "    output [3:0] o2\n"
      ");\n"
      "wire [4:0] x;\n"
      "wire [4:0] y;\n"
      "wire [4:0] h;\n"
      "wire [4:0] g;\n"
      "assign x = i1 + i2;\n"
      "assign h = i1 - i2;\n"
      "assign g = h;\n"
      "assign o0 = x[3:0];\n"
      "assign y = i1;\n"
      "assign o1 = y[3:0];\n"
      "assign o2 = g[3:0];\n"
      "endmodule\n";

  EXPECT_EQ(module->toString(), raw_str);

  std::string expected_str =
      "module test_module (\n"
      "    input [4:0] i1,\n"
      "    input [4:0] i2,\n"
      "    output [3:0] o0,\n"
      "    output [3:0] o1,\n"
      "    output [3:0] o2\n"
      ");\n"
      "wire [4:0] x;\n"
      "wire [4:0] h;\n"
      "assign x = i1 + i2;\n"
      "assign h = i1 - i2;\n"
      "assign o0 = x[3:0];\n"
      "assign o1 = i1[3:0];\n"
      "assign o2 = h[3:0];\n"
      "endmodule\n";

  vAST::AssignInliner transformer;
  EXPECT_EQ(transformer.visit(std::move(module))->toString(), expected_str);
}

TEST(InlineAssignTests, TestInlineNestedSliceIndex) {
  std::vector<std::unique_ptr<vAST::AbstractPort>> ports;
  std::vector<std::pair<std::unique_ptr<vAST::Expression>,
                        std::unique_ptr<vAST::Expression>>>
      outer_dims;
  outer_dims.push_back({vAST::make_num("7"), vAST::make_num("0")});
  ports.push_back(std::make_unique<vAST::Port>(
      std::make_unique<vAST::NDVector>(vAST::make_id("i"), vAST::make_num("4"),
                                       vAST::make_num("0"),
                                       std::move(outer_dims)),
      vAST::INPUT, vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(
      std::make_unique<vAST::Vector>(vAST::make_id("o"), vAST::make_num("3"),
                                     vAST::make_num("0")),
      vAST::OUTPUT, vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("o_2"),
                                               vAST::OUTPUT, vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(
      std::make_unique<vAST::Vector>(vAST::make_id("o_3"), vAST::make_num("4"),
                                     vAST::make_num("0")),
      vAST::OUTPUT, vAST::WIRE));
  std::vector<std::pair<std::unique_ptr<vAST::Expression>,
                        std::unique_ptr<vAST::Expression>>>
      o_4_dims;
  o_4_dims.push_back({vAST::make_num("3"), vAST::make_num("0")});
  ports.push_back(std::make_unique<vAST::Port>(
      std::make_unique<vAST::NDVector>(vAST::make_id("o_4"),
                                       vAST::make_num("4"), vAST::make_num("0"),
                                       std::move(o_4_dims)),
      vAST::OUTPUT, vAST::WIRE));

  std::vector<std::variant<std::unique_ptr<vAST::StructuralStatement>,
                           std::unique_ptr<vAST::Declaration>>>
      body;

  body.push_back(std::make_unique<vAST::Wire>(std::make_unique<vAST::Vector>(
      vAST::make_id("x"), vAST::make_num("4"), vAST::make_num("0"))));

  body.push_back(std::make_unique<vAST::Wire>(std::make_unique<vAST::Vector>(
      vAST::make_id("y"), vAST::make_num("4"), vAST::make_num("0"))));

  std::vector<std::pair<std::unique_ptr<vAST::Expression>,
                        std::unique_ptr<vAST::Expression>>>
      g_dims;
  g_dims.push_back({vAST::make_num("3"), vAST::make_num("0")});
  body.push_back(std::make_unique<vAST::Wire>(std::make_unique<vAST::NDVector>(
      vAST::make_id("g"), vAST::make_num("4"), vAST::make_num("0"),
      std::move(g_dims))));

  std::vector<std::pair<std::unique_ptr<vAST::Expression>,
                        std::unique_ptr<vAST::Expression>>>
      h_dims;
  h_dims.push_back({vAST::make_num("6"), vAST::make_num("0")});
  body.push_back(std::make_unique<vAST::Wire>(std::make_unique<vAST::NDVector>(
      vAST::make_id("h"), vAST::make_num("4"), vAST::make_num("0"),
      std::move(h_dims))));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      vAST::make_id("x"), std::make_unique<vAST::Index>(
                              std::make_unique<vAST::Identifier>("i"),
                              std::make_unique<vAST::NumericLiteral>("0"))));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      vAST::make_id("y"), std::make_unique<vAST::Index>(
                              std::make_unique<vAST::Identifier>("i"),
                              std::make_unique<vAST::NumericLiteral>("1"))));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      vAST::make_id("g"),
      std::make_unique<vAST::Slice>(vAST::make_id("i"), vAST::make_num("3"),
                                    vAST::make_num("0"))));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      vAST::make_id("h"),
      std::make_unique<vAST::Slice>(vAST::make_id("i"), vAST::make_num("6"),
                                    vAST::make_num("0"))));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      vAST::make_id("o"),
      std::make_unique<vAST::Slice>(vAST::make_id("x"), vAST::make_num("3"),
                                    vAST::make_num("0"))));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      vAST::make_id("o_2"),
      std::make_unique<vAST::Index>(vAST::make_id("y"), vAST::make_num("0"))));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      vAST::make_id("o_3"),
      std::make_unique<vAST::Index>(vAST::make_id("g"), vAST::make_num("0"))));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      vAST::make_id("o_4"),
      std::make_unique<vAST::Slice>(vAST::make_id("h"), vAST::make_num("3"),
                                    vAST::make_num("0"))));

  std::unique_ptr<vAST::AbstractModule> module = std::make_unique<vAST::Module>(
      "test_module", std::move(ports), std::move(body));

  std::string raw_str =
      "module test_module (\n"
      "    input [4:0] i [7:0],\n"
      "    output [3:0] o,\n"
      "    output o_2,\n"
      "    output [4:0] o_3,\n"
      "    output [4:0] o_4 [3:0]\n"
      ");\n"
      "wire [4:0] x;\n"
      "wire [4:0] y;\n"
      "wire [4:0] g [3:0];\n"
      "wire [4:0] h [6:0];\n"
      "assign x = i[0];\n"
      "assign y = i[1];\n"
      "assign g = i[3:0];\n"
      "assign h = i[6:0];\n"
      "assign o = x[3:0];\n"
      "assign o_2 = y[0];\n"
      "assign o_3 = g[0];\n"
      "assign o_4 = h[3:0];\n"
      "endmodule\n";

  EXPECT_EQ(module->toString(), raw_str);

  std::string expected_str =
      "module test_module (\n"
      "    input [4:0] i [7:0],\n"
      "    output [3:0] o,\n"
      "    output o_2,\n"
      "    output [4:0] o_3,\n"
      "    output [4:0] o_4 [3:0]\n"
      ");\n"
      "assign o = i[0][3:0];\n"
      "assign o_2 = i[1][0];\n"
      "assign o_3 = i[3:0][0];\n"
      "assign o_4 = i[6:0][3:0];\n"
      "endmodule\n";

  vAST::AssignInliner transformer;
  EXPECT_EQ(transformer.visit(std::move(module))->toString(), expected_str);
}

TEST(InlineAssignTests, TestInlineMultipleAssign) {
  std::vector<std::unique_ptr<vAST::AbstractPort>> ports;
  ports.push_back(std::make_unique<vAST::Port>(
      std::make_unique<vAST::Vector>(vAST::make_id("i0"), vAST::make_num("4"),
                                     vAST::make_num("0")),
      vAST::INPUT, vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(
      std::make_unique<vAST::Vector>(vAST::make_id("i1"), vAST::make_num("4"),
                                     vAST::make_num("0")),
      vAST::INPUT, vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("s"), vAST::INPUT,
                                               vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(
      std::make_unique<vAST::Vector>(vAST::make_id("o"), vAST::make_num("3"),
                                     vAST::make_num("0")),
      vAST::OUTPUT, vAST::WIRE));

  std::vector<std::variant<std::unique_ptr<vAST::StructuralStatement>,
                           std::unique_ptr<vAST::Declaration>>>
      body;

  body.push_back(std::make_unique<vAST::Wire>(std::make_unique<vAST::Vector>(
      vAST::make_id("x"), vAST::make_num("4"), vAST::make_num("0"))));

  body.push_back(std::make_unique<vAST::Wire>(std::make_unique<vAST::Vector>(
      vAST::make_id("y"), vAST::make_num("4"), vAST::make_num("0"))));

  body.push_back(std::make_unique<vAST::Wire>(std::make_unique<vAST::Vector>(
      vAST::make_id("z"), vAST::make_num("4"), vAST::make_num("0"))));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(vAST::make_id("x"),
                                                          vAST::make_id("i0")));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(vAST::make_id("y"),
                                                          vAST::make_id("i1")));

  std::vector<std::variant<
      std::unique_ptr<vAST::Identifier>, std::unique_ptr<vAST::PosEdge>,
      std::unique_ptr<vAST::NegEdge>, std::unique_ptr<vAST::Star>>>
      sensitivity_list;
  sensitivity_list.push_back(std::make_unique<vAST::Star>());

  std::vector<std::unique_ptr<vAST::BehavioralStatement>> always_body;

  std::vector<std::unique_ptr<vAST::BehavioralStatement>> true_body;
  true_body.push_back(std::make_unique<vAST::BlockingAssign>(
      std::make_unique<vAST::Identifier>("z"),
      std::make_unique<vAST::Identifier>("x")));

  std::vector<std::unique_ptr<vAST::BehavioralStatement>> else_body;
  else_body.push_back(std::make_unique<vAST::BlockingAssign>(
      std::make_unique<vAST::Identifier>("z"),
      std::make_unique<vAST::Identifier>("y")));

  always_body.push_back(std::make_unique<vAST::If>(
      vAST::make_id("s"), std::move(true_body), std::move(else_body)));

  body.push_back(std::make_unique<vAST::Always>(std::move(sensitivity_list),
                                                std::move(always_body)));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      vAST::make_id("o"),
      std::make_unique<vAST::Slice>(vAST::make_id("z"), vAST::make_num("3"),
                                    vAST::make_num("0"))));

  std::unique_ptr<vAST::AbstractModule> module = std::make_unique<vAST::Module>(
      "test_module", std::move(ports), std::move(body));

  std::string raw_str =
      "module test_module (\n"
      "    input [4:0] i0,\n"
      "    input [4:0] i1,\n"
      "    input s,\n"
      "    output [3:0] o\n"
      ");\n"
      "wire [4:0] x;\n"
      "wire [4:0] y;\n"
      "wire [4:0] z;\n"
      "assign x = i0;\n"
      "assign y = i1;\n"
      "always @(*) begin\n"
      "if (s) begin\n"
      "    z = x;\n"
      "end else begin\n"
      "    z = y;\n"
      "end\n"
      "end\n"
      "\n"
      "assign o = z[3:0];\n"
      "endmodule\n";

  EXPECT_EQ(module->toString(), raw_str);

  std::string expected_str =
      "module test_module (\n"
      "    input [4:0] i0,\n"
      "    input [4:0] i1,\n"
      "    input s,\n"
      "    output [3:0] o\n"
      ");\n"
      "wire [4:0] z;\n"
      "always @(*) begin\n"
      "if (s) begin\n"
      "    z = i0;\n"
      "end else begin\n"
      "    z = i1;\n"
      "end\n"
      "end\n"
      "\n"
      "assign o = z[3:0];\n"
      "endmodule\n";

  vAST::AssignInliner transformer;
  EXPECT_EQ(transformer.visit(std::move(module))->toString(), expected_str);
}

TEST(InlineAssignTests, TestNoInlineNumToIndexSlice) {
  std::vector<std::unique_ptr<vAST::AbstractPort>> ports;
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("o0"),
                                               vAST::OUTPUT, vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(
      std::make_unique<vAST::Vector>(vAST::make_id("o1"), vAST::make_num("1"),
                                     vAST::make_num("0")),
      vAST::OUTPUT, vAST::WIRE));

  std::vector<std::variant<std::unique_ptr<vAST::StructuralStatement>,
                           std::unique_ptr<vAST::Declaration>>>
      body;

  body.push_back(std::make_unique<vAST::Wire>(std::make_unique<vAST::Vector>(
      vAST::make_id("x"), vAST::make_num("2"), vAST::make_num("0"))));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(vAST::make_id("x"),
                                                          vAST::make_num("7")));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      vAST::make_id("o0"),
      std::make_unique<vAST::Index>(vAST::make_id("x"), vAST::make_num("0"))));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      vAST::make_id("o1"),
      std::make_unique<vAST::Slice>(vAST::make_id("x"), vAST::make_num("1"),
                                    vAST::make_num("0"))));

  std::unique_ptr<vAST::AbstractModule> module = std::make_unique<vAST::Module>(
      "test_module", std::move(ports), std::move(body));

  std::string raw_str =
      "module test_module (\n"
      "    output o0,\n"
      "    output [1:0] o1\n"
      ");\n"
      "wire [2:0] x;\n"
      "assign x = 7;\n"
      "assign o0 = x[0];\n"
      "assign o1 = x[1:0];\n"
      "endmodule\n";

  EXPECT_EQ(module->toString(), raw_str);

  std::string expected_str = raw_str;

  vAST::AssignInliner transformer;
  EXPECT_EQ(transformer.visit(std::move(module))->toString(), expected_str);
}

}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
