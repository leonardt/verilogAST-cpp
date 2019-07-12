#include "common.cpp"
#include "gtest/gtest.h"
#include "verilogAST.hpp"

using namespace std;

namespace vAST = verilogAST;

/*
module coreir_eq #(parameter width=1) (
  input [width-1:0] in0,
  input [width-1:0] in1,
  output [width-1:0] out
);
  assign out = in0 & in1;

endmodule  // coreir_eq
*/

namespace {

TEST(ParameterizedModuleTests, TestEq) {
  std::string name = "coreir_eq";

  std::vector<std::unique_ptr<vAST::AbstractPort>> ports;
  ports.push_back(make_port(
      make_vector(make_id("in0"),
                  make_binop(make_id("width"), vAST::BinOp::SUB, make_num("1")),
                  make_num("0")),
      vAST::INPUT, vAST::WIRE));
  ports.push_back(make_port(
      make_vector(make_id("in1"),
                  make_binop(make_id("width"), vAST::BinOp::SUB, make_num("1")),
                  make_num("0")),
      vAST::INPUT, vAST::WIRE));
  ports.push_back(make_port(make_id("out"), vAST::OUTPUT, vAST::WIRE));

  std::vector<std::variant<std::unique_ptr<vAST::StructuralStatement>,
                           std::unique_ptr<vAST::Declaration>>>
      body;

  std::unique_ptr<vAST::ContinuousAssign> eq_assign =
      std::make_unique<vAST::ContinuousAssign>(
          make_id("out"),
          make_binop(make_id("in0"), vAST::BinOp::EQ, make_id("in1")));

  body.push_back(std::move(eq_assign));

  vAST::Parameters parameters;
  parameters.push_back(std::make_pair(make_id("width"), make_num("1")));
  std::unique_ptr<vAST::Module> coreir_eq = std::make_unique<vAST::Module>(
      name, std::move(ports), std::move(body), std::move(parameters));

  cout << "//coreir_eq" << endl << coreir_eq->toString() << endl;
  std::string expected_str =
      "module coreir_eq #(parameter width = 1) (input [width - 1:0] in0, input "
      "[width - 1:0] in1, output out);\n"
      "assign out = in0 == in1;\n"
      "endmodule\n";
  EXPECT_EQ(coreir_eq->toString(), expected_str);
}

}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
