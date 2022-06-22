#include "verilogAST/make_packed.hpp"
#include "common.cpp"
#include "gtest/gtest.h"

namespace vAST = verilogAST;

namespace {

using BodyElement = std::variant<std::unique_ptr<vAST::StructuralStatement>,
                                 std::unique_ptr<vAST::Declaration>>;
using Dim = std::pair<std::unique_ptr<vAST::Expression>,
                      std::unique_ptr<vAST::Expression>>;


Dim makeDim(std::unique_ptr<vAST::Expression> hi,
            std::unique_ptr<vAST::Expression> lo) {
  return std::make_pair(std::move(hi), std::move(lo));
}

TEST(MakePackedTests, TestBasic) {
  std::vector<std::unique_ptr<vAST::AbstractPort>> ports;

  auto make_dims = []() {
    std::vector<Dim> dims;
    dims.push_back(makeDim(vAST::make_num("7"), vAST::make_num("0")));
    dims.push_back(makeDim(vAST::make_num("15"), vAST::make_num("0")));
    return dims;
  };
  ports.push_back(
      std::make_unique<vAST::Port>(
          std::make_unique<vAST::NDVector>(
              vAST::make_id("I"),
              vAST::make_num("31"),
              vAST::make_num("0"),
              make_dims()),
          vAST::INPUT,
          vAST::WIRE));

  std::vector<BodyElement> body;

  std::unique_ptr<vAST::AbstractModule> module = std::make_unique<vAST::Module>(
      "test_module",
      std::move(ports),
      std::move(body));

  auto pre =
      "module test_module (\n"
      "    input [31:0] I [7:0][15:0]\n"
      ");\n"
      "endmodule\n";
  EXPECT_EQ(module->toString(), pre);

  // Run MakePacked transformer.
  vAST::MakePacked transformer {};
  module = transformer.visit(std::move(module));

  auto post =
      "module test_module (\n"
      "    input [7:0][15:0][31:0] I\n"
      ");\n"
      "endmodule\n";
  EXPECT_EQ(module->toString(), post);
}

}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
