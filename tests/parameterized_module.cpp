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

  std::unique_ptr<vAST::Identifier> in0 =
      std::make_unique<vAST::Identifier>("in0");
  std::unique_ptr<vAST::Identifier> in1 =
      std::make_unique<vAST::Identifier>("in1");
  std::unique_ptr<vAST::Identifier> out =
      std::make_unique<vAST::Identifier>("out");

  std::unique_ptr<vAST::Vector> in0_vec = std::make_unique<vAST::Vector>(
      std::make_unique<vAST::Identifier>("in0"),
      std::make_unique<vAST::BinaryOp>(std::make_unique<vAST::Identifier>("width"), vAST::BinOp::SUB,
                                       std::make_unique<vAST::NumericLiteral>("1")),
      std::make_unique<vAST::NumericLiteral>("0"));
  std::unique_ptr<vAST::Vector> in1_vec = std::make_unique<vAST::Vector>(
      std::make_unique<vAST::Identifier>("in1"),
      std::make_unique<vAST::BinaryOp>(std::make_unique<vAST::Identifier>("width"), vAST::BinOp::SUB,
                                       std::make_unique<vAST::NumericLiteral>("1")),
      std::make_unique<vAST::NumericLiteral>("0"));

  std::unique_ptr<vAST::Port> in0_port =
      std::make_unique<vAST::Port>(std::move(in0_vec), vAST::INPUT, vAST::WIRE);
  std::unique_ptr<vAST::Port> in1_port =
      std::make_unique<vAST::Port>(std::move(in1_vec), vAST::INPUT, vAST::WIRE);
  std::unique_ptr<vAST::Port> out_port =
      std::make_unique<vAST::Port>(std::make_unique<vAST::Identifier>("out"), vAST::OUTPUT, vAST::WIRE);

  std::vector<std::unique_ptr<vAST::AbstractPort>> ports;
  ports.push_back(std::move(in0_port)); 
  ports.push_back(std::move(in1_port));
  ports.push_back(std::move(out_port));

  std::vector<std::variant<std::unique_ptr<vAST::StructuralStatement>,
                           std::unique_ptr<vAST::Declaration>>>
      body;

  std::unique_ptr<vAST::BinaryOp> eq_op = std::make_unique<vAST::BinaryOp>(
      std::make_unique<vAST::Identifier>("in0"), vAST::BinOp::EQ, std::make_unique<vAST::Identifier>("in1"));
  std::unique_ptr<vAST::ContinuousAssign> eq_assign =
      std::make_unique<vAST::ContinuousAssign>(std::make_unique<vAST::Identifier>("out"),
                                               std::move(eq_op));

  body.push_back(std::move(eq_assign));

  std::unique_ptr<vAST::Identifier> width =
      std::make_unique<vAST::Identifier>("width");
  std::unique_ptr<vAST::NumericLiteral> one =
      std::make_unique<vAST::NumericLiteral>("1");
  vAST::Parameters parameters;
  parameters.push_back(std::make_pair(std::move(width), std::move(one)));
  std::unique_ptr<vAST::Module> coreir_eq =
      std::make_unique<vAST::Module>(name, std::move(ports), std::move(body), std::move(parameters));

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
