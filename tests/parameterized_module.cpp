#include "gtest/gtest.h"
#include "verilogAST.hpp"

namespace vAST = verilogAST;

/*
module coreir_xor #(parameter width=1) (
  input [width-1:0] in0,
  input [width-1:0] in1,
  output [width-1:0] out
);
  assign out = in0 ^ in1;

endmodule  // coreir_xor
*/

namespace {
TEST(ParamTests, TestXor) {
  std::string name = "coreir_xor";
  
  vAST::Parameter width("width");
  vAST::Identifier in0("in0");
  vAST::Identifier in1("in1");
  vAST::Identifier out("out");
  vAST::NumericLiteral one("1");
  vAST::NumericLiteral zero("0");
  vAST::ConstantBinaryOp hi(&width,vAST::BinOp::SUB,&one);
  auto lo = zero;

  vAST::Slice in0_slice(&in0, &hi, &lo);
  vAST::Slice in1_slice(&in1, &hi, &lo);
  vAST::Slice out_slice(&out, &hi, &lo);

  vAST::Port in0_port(&in0_slice, vAST::INPUT, vAST::WIRE);
  vAST::Port in1_port(&in1_slice, vAST::INPUT, vAST::WIRE);
  vAST::Port out_port(&out_slice, vAST::OUTPUT, vAST::WIRE);

  std::vector<vAST::Port *> ports = {&in0_port, &in1_port, &out_port};
  
  vAST::Identifier a("a");
  vAST::Identifier b("b");
  vAST::ContinuousAssign cont_assign(&a, &b);

  std::vector<std::variant<vAST::Always *, vAST::StructuralStatement *,
                           vAST::Declaration *>>
      body;

  std::string module_name = "other_module";

  vAST::NumericLiteral zero("0");
  vAST::NumericLiteral one("1");

  std::map<std::string, vAST::NumericLiteral *> inst_parameters = {
      {"param0", &zero}, {"param1", &one}};

  std::string instance_name = "other_module_inst";
  vAST::Identifier a("a");
  vAST::Identifier b("b");
  vAST::Index b_index(&b, &zero);
  vAST::Identifier c("c");
  vAST::NumericLiteral high("31");
  vAST::NumericLiteral low("0");
  vAST::Slice c_slice(&c, &high, &low);

  std::map<std::string,
           std::variant<vAST::Identifier *, vAST::Index *, vAST::Slice *>>
      connections = {{"a", &a}, {"b", &b_index}, {"c", &c_slice}};

  vAST::ModuleInstantiation module_inst(module_name, inst_parameters,
                                        instance_name, connections);
  body.push_back(&module_inst);

  std::map<std::string, vAST::NumericLiteral *> parameters;
  vAST::Module module(name, ports, body, parameters);

  std::string expected_str =
      "module test_module (input i, output o);\nother_module #(.param0(32'd0), "
      ".param1(32'd1)) other_module_inst(.a(a), .b(b[32'd0]), "
      ".c(c[32'd31:32'd0]));\nendmodule\n";
  EXPECT_EQ(module.toString(), expected_str);

  parameters = {{"param0", &zero}, {"param1", &one}};
  vAST::Module module_with_params(name, ports, body, parameters);

  expected_str =
      "module test_module #(parameter param0 = 32'd0, parameter param1 = "
      "32'd1) (input i, output o);\nother_module #(.param0(32'd0), "
      ".param1(32'd1)) other_module_inst(.a(a), .b(b[32'd0]), "
      ".c(c[32'd31:32'd0]));\nendmodule\n";
  EXPECT_EQ(module_with_params.toString(), expected_str);
}
