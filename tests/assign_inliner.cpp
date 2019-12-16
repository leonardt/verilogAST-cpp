#include "common.cpp"
#include "gtest/gtest.h"
#include "verilogAST/transformer.hpp"

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

  std::vector<std::variant<std::unique_ptr<vAST::StructuralStatement>,
                           std::unique_ptr<vAST::Declaration>>>
      body;

  body.push_back(
      std::make_unique<vAST::Wire>(std::make_unique<vAST::Identifier>("x")));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("x"),
      std::make_unique<vAST::Identifier>("i")));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("o"),
      std::make_unique<vAST::Identifier>("x")));

  std::unique_ptr<vAST::AbstractModule> module = std::make_unique<vAST::Module>(
      "test_module", std::move(ports), std::move(body));

  std::string raw_str =
      "module test_module (input i, output o);\n"
      "wire x;\n"
      "assign x = i;\n"
      "assign o = x;\n"
      "endmodule\n";

  EXPECT_EQ(module->toString(), raw_str);

  std::string expected_str =
      "module test_module (input i, output o);\n"
      "assign o = i;\n"
      "endmodule\n";

  vAST::AssignInliner transformer;
  EXPECT_EQ(transformer.visit(std::move(module))->toString(), expected_str);
}

TEST(InlineAssignTests, TestIndex) {
  std::vector<std::unique_ptr<vAST::AbstractPort>> ports;
  ports.push_back(std::make_unique<vAST::Port>(
      std::make_unique<vAST::Vector>(vAST::make_id("i"), vAST::make_num("1"),
                                     vAST::make_num("0")),
      vAST::INPUT, vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(
      std::make_unique<vAST::Vector>(vAST::make_id("o"), vAST::make_num("1"),
                                     vAST::make_num("0")),
      vAST::OUTPUT, vAST::WIRE));

  std::vector<std::variant<std::unique_ptr<vAST::StructuralStatement>,
                           std::unique_ptr<vAST::Declaration>>>
      body;

  body.push_back(std::make_unique<vAST::Wire>(std::make_unique<vAST::Vector>(
      vAST::make_id("x"), vAST::make_num("1"), vAST::make_num("0"))));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Index>(std::make_unique<vAST::Identifier>("x"),
                                    vAST::make_num("0")),
      std::make_unique<vAST::Index>(std::make_unique<vAST::Identifier>("i"),
                                    vAST::make_num("0"))));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Index>(std::make_unique<vAST::Identifier>("x"),
                                    vAST::make_num("1")),
      std::make_unique<vAST::Index>(std::make_unique<vAST::Identifier>("i"),
                                    vAST::make_num("1"))));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Index>(std::make_unique<vAST::Identifier>("o"),
                                    vAST::make_num("0")),
      std::make_unique<vAST::Index>(std::make_unique<vAST::Identifier>("x"),
                                    vAST::make_num("0"))));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Index>(std::make_unique<vAST::Identifier>("o"),
                                    vAST::make_num("1")),
      std::make_unique<vAST::Index>(std::make_unique<vAST::Identifier>("x"),
                                    vAST::make_num("1"))));

  std::unique_ptr<vAST::AbstractModule> module = std::make_unique<vAST::Module>(
      "test_module", std::move(ports), std::move(body));

  std::string raw_str =
      "module test_module (input [1:0] i, output [1:0] o);\n"
      "wire [1:0] x;\n"
      "assign x[0] = i[0];\n"
      "assign x[1] = i[1];\n"
      "assign o[0] = x[0];\n"
      "assign o[1] = x[1];\n"
      "endmodule\n";
  EXPECT_EQ(module->toString(), raw_str);

  std::string expected_str =
      "module test_module (input [1:0] i, output [1:0] o);\n"
      "assign o[0] = i[0];\n"
      "assign o[1] = i[1];\n"
      "endmodule\n";

  vAST::AssignInliner transformer;
  EXPECT_EQ(transformer.visit(std::move(module))->toString(), expected_str);
}
}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
