#include "verilogAST/zext_coalescer.hpp"
#include "common.cpp"
#include "gtest/gtest.h"

namespace vAST = verilogAST;

namespace {

using BodyElement = std::variant<std::unique_ptr<vAST::StructuralStatement>,
                                 std::unique_ptr<vAST::Declaration>>;

auto makeVector(std::string name, int hi, int lo) {
  return std::make_unique<vAST::Vector>(vAST::make_id(name),
                                        vAST::make_num(std::to_string(hi)),
                                        vAST::make_num(std::to_string(lo)));
}

void runTest(std::vector<int> prefix,
             std::string pre,
             std::string post,
             bool elide = false) {
  std::vector<std::unique_ptr<vAST::AbstractPort>> ports;
  ports.push_back(std::make_unique<vAST::Port>(makeVector("I", 7, 0),
                                               vAST::INPUT,
                                               vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(makeVector("O", 7, 0),
                                               vAST::OUTPUT,
                                               vAST::WIRE));
  std::vector<BodyElement> body;

  std::vector<std::unique_ptr<vAST::Expression>> args = {};
  int remaining_size = 8;
  for (const auto& it : prefix) {
    args.emplace_back(new vAST::NumericLiteral("0", it));
    remaining_size -= it;
  }
  args.emplace_back(
      new vAST::Slice(
          vAST::make_id("I"),
          vAST::make_num(std::to_string(remaining_size - 1)),
          vAST::make_num("0")));
  std::unique_ptr<vAST::Expression> rhs(new vAST::Concat(std::move(args)));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("O"), std::move(rhs)));

  std::unique_ptr<vAST::AbstractModule> module = std::make_unique<vAST::Module>(
      "test_module",
      std::move(ports),
      std::move(body));

  EXPECT_EQ(module->toString(), pre);

  // Run ZextCoalescer transformer.
  vAST::ZextCoalescer transformer(elide);
  module = transformer.visit(std::move(module));

  EXPECT_EQ(module->toString(), post);
}

TEST(ZextCoalescerTests, TestBasic) {
  auto pre =
      "module test_module (\n"
      "    input [7:0] I,\n"
      "    output [7:0] O\n"
      ");\n"
      "assign O = {1'd0,2'd0,I[4:0]};\n"
      "endmodule\n";
  auto post =
      "module test_module (\n"
      "    input [7:0] I,\n"
      "    output [7:0] O\n"
      ");\n"
      "assign O = {3'd0,I[4:0]};\n"
      "endmodule\n";
  runTest({1, 2}, pre, post);
}

TEST(ZextCoalescerTests, TestElide) {
  auto pre =
      "module test_module (\n"
      "    input [7:0] I,\n"
      "    output [7:0] O\n"
      ");\n"
      "assign O = {1'd0,2'd0,I[4:0]};\n"
      "endmodule\n";
  auto post =
      "module test_module (\n"
      "    input [7:0] I,\n"
      "    output [7:0] O\n"
      ");\n"
      "assign O = {I[4:0]};\n"
      "endmodule\n";
  runTest({1, 2}, pre, post, true /* elide */);
}

TEST(ZextCoalescerTests, TestNoop) {
  auto pre =
      "module test_module (\n"
      "    input [7:0] I,\n"
      "    output [7:0] O\n"
      ");\n"
      "assign O = {I[7:0]};\n"
      "endmodule\n";
  auto post =
      "module test_module (\n"
      "    input [7:0] I,\n"
      "    output [7:0] O\n"
      ");\n"
      "assign O = {I[7:0]};\n"
      "endmodule\n";
  runTest({}, pre, post);
}

}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
