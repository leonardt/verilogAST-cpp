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
  
  vAST::Identifier width("width");
  vAST::Identifier in0("in0");
  vAST::Identifier in1("in1");
  vAST::Identifier out("out");
  vAST::NumericLiteral one("1");
  vAST::NumericLiteral zero("0");
  vAST::BinaryOp hi(&width,vAST::BinOp::SUB,&one);
  auto lo = zero;

  vAST::Slice in0_slice(&in0, &hi, &lo);
  vAST::Slice in1_slice(&in1, &hi, &lo);

  vAST::Port in0_port(&in0_slice, vAST::INPUT, vAST::WIRE);
  vAST::Port in1_port(&in1_slice, vAST::INPUT, vAST::WIRE);
  vAST::Port out_port(&out, vAST::OUTPUT, vAST::WIRE);

  std::vector<vAST::Port *> ports = {&in0_port, &in1_port, &out_port};

  std::vector<std::variant<vAST::StructuralStatement *, vAST::Declaration *>>
      body;

  vAST::BinaryOp eq_op(&in0,vAST::BinOp::EQ,&in1);
  vAST::ContinuousAssign eq_assign(&out, &eq_op);

  body.push_back(&eq_assign);

  vAST::Parameters parameters = {{&width,&one}};
  vAST::Module coreir_eq(name, ports, body, parameters);

  cout << "//coreir_eq" << endl << coreir_eq.toString() << endl;
  std::string expected_str =
      "module coreir_eq #(parameter width = 32'd1) (input in0[width - 32'd1:32'd0], input in1[width - 32'd1:32'd0], output out);\n"
      "assign out = in0 == in1;\n"
      "endmodule\n";
  EXPECT_EQ(coreir_eq.toString(), expected_str);
}

} // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
