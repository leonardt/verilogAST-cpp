#include "verilogAST/concat_coalescer.hpp"
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

TEST(ConcatCoalescerTests, TestBasic) {
  std::vector<std::unique_ptr<vAST::AbstractPort>> ports;
  ports.push_back(std::make_unique<vAST::Port>(makeVector("I", 7, 0),
                                               vAST::INPUT,
                                               vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(makeVector("O0", 3, 0),
                                               vAST::OUTPUT,
                                               vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(makeVector("O1", 3, 0),
                                               vAST::OUTPUT,
                                               vAST::WIRE));
  std::vector<BodyElement> body;

  std::vector<std::unique_ptr<vAST::Expression>> args0 = {};
  std::vector<std::unique_ptr<vAST::Expression>> args1 = {};
  for (int i = 3; i >= 0; i--) {
    args0.emplace_back(new vAST::Index(vAST::make_id("I"),
                                       vAST::make_num(std::to_string(i))));
    args1.emplace_back(new vAST::Index(vAST::make_id("I"),
                                       vAST::make_num(std::to_string(i + 4))));
  }
  std::unique_ptr<vAST::Expression> rhs0(new vAST::Concat(std::move(args0)));
  std::unique_ptr<vAST::Expression> rhs1(new vAST::Concat(std::move(args1)));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("O0"), std::move(rhs0)));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("O1"), std::move(rhs1)));

  std::unique_ptr<vAST::AbstractModule> module = std::make_unique<vAST::Module>(
      "test_module",
      std::move(ports),
      std::move(body));

  std::string expected;

  expected =
      "module test_module (\n"
      "    input [7:0] I,\n"
      "    output [3:0] O0,\n"
      "    output [3:0] O1\n"
      ");\n"
      "assign O0 = {I[3],I[2],I[1],I[0]};\n"
      "assign O1 = {I[7],I[6],I[5],I[4]};\n"
      "endmodule\n";
  EXPECT_EQ(module->toString(), expected);

  // Run ConcatCoalescer transformer.
  vAST::ConcatCoalescer transformer;
  module = transformer.visit(std::move(module));
  expected =
      "module test_module (\n"
      "    input [7:0] I,\n"
      "    output [3:0] O0,\n"
      "    output [3:0] O1\n"
      ");\n"
      "assign O0 = I[3:0];\n"
      "assign O1 = I[7:4];\n"
      "endmodule\n";
  EXPECT_EQ(module->toString(), expected);
}

}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
