#include "gtest/gtest.h"
#include "verilogAST.hpp"

namespace vAST = verilogAST;

namespace {

TEST(BasicTests, TestNumericLiteral) {
  vAST::NumericLiteral n0("23", 16, false, vAST::DECIMAL);
  EXPECT_EQ(n0.toString(), "16'd23");

  vAST::NumericLiteral n1("DEADBEEF", 32, false, vAST::HEX);
  EXPECT_EQ(n1.toString(), "32'hDEADBEEF");

  vAST::NumericLiteral n2("011001", 6, false, vAST::BINARY);
  EXPECT_EQ(n2.toString(), "6'b011001");

  vAST::NumericLiteral n3("764", 24, false, vAST::OCTAL);
  EXPECT_EQ(n3.toString(), "24'o764");

  vAST::NumericLiteral n4("764", 8, false);
  EXPECT_EQ(n4.toString(), "8'd764");

  vAST::NumericLiteral n5("764", 8);
  EXPECT_EQ(n5.toString(), "8'd764");

  vAST::NumericLiteral n6("764");
  EXPECT_EQ(n6.toString(), "32'd764");

  vAST::NumericLiteral n7("764", 8, true);
  EXPECT_EQ(n7.toString(), "8'sd764");
}

TEST(BasicTests, TestIdentifier) {
  vAST::Identifier id("x");
  EXPECT_EQ(id.toString(), "x");
}

TEST(BasicTests, TestString) {
  vAST::String str("mystring");
  EXPECT_EQ(str.toString(), "\"mystring\"");
}

TEST(BasicTests, TestIndex) {
  vAST::Identifier id("x");
  vAST::NumericLiteral n("0");
  vAST::Index index(&id, &n);
  EXPECT_EQ(index.toString(), "x[32'd0]");
}

TEST(BasicTests, TestSlice) {
  vAST::Identifier id("x");
  vAST::NumericLiteral high("31");
  vAST::NumericLiteral low("0");
  vAST::Slice slice(&id, &high, &low);
  EXPECT_EQ(slice.toString(), "x[32'd31:32'd0]");
}

TEST(BasicTests, TestVector) {
  vAST::Identifier id("x");
  vAST::NumericLiteral high("31");
  vAST::NumericLiteral low("0");
  vAST::Vector slice(&id, &high, &low);
  EXPECT_EQ(slice.toString(), "[32'd31:32'd0] x");
}

TEST(BasicTests, TestBinaryOp) {
  std::vector<std::pair<vAST::BinOp::BinOp, std::string>> ops;
  ops.push_back(std::make_pair(vAST::BinOp::LSHIFT, "<<"));
  ops.push_back(std::make_pair(vAST::BinOp::RSHIFT, ">>"));
  ops.push_back(std::make_pair(vAST::BinOp::AND, "&&"));
  ops.push_back(std::make_pair(vAST::BinOp::OR, "||"));
  ops.push_back(std::make_pair(vAST::BinOp::EQ, "=="));
  ops.push_back(std::make_pair(vAST::BinOp::NEQ, "!="));
  ops.push_back(std::make_pair(vAST::BinOp::ADD, "+"));
  ops.push_back(std::make_pair(vAST::BinOp::SUB, "-"));
  ops.push_back(std::make_pair(vAST::BinOp::MUL, "*"));
  ops.push_back(std::make_pair(vAST::BinOp::DIV, "/"));
  ops.push_back(std::make_pair(vAST::BinOp::POW, "**"));
  ops.push_back(std::make_pair(vAST::BinOp::MOD, "%"));
  ops.push_back(std::make_pair(vAST::BinOp::ALSHIFT, "<<<"));
  ops.push_back(std::make_pair(vAST::BinOp::ARSHIFT, ">>>"));
  vAST::Identifier x("x");
  vAST::Identifier y("y");
  for (auto it : ops) {
    vAST::BinOp::BinOp op = it.first;
    std::string op_str = it.second;
    vAST::BinaryOp bin_op(&x, op, &y);
    EXPECT_EQ(bin_op.toString(), "x " + op_str + " y");
  }
}

TEST(BasicTests, TestUnaryOp) {
  std::vector<std::pair<vAST::UnOp::UnOp, std::string>> ops;
  ops.push_back(std::make_pair(vAST::UnOp::NOT, "!"));
  ops.push_back(std::make_pair(vAST::UnOp::INVERT, "~"));
  ops.push_back(std::make_pair(vAST::UnOp::AND, "&"));
  ops.push_back(std::make_pair(vAST::UnOp::NAND, "~&"));
  ops.push_back(std::make_pair(vAST::UnOp::OR, "|"));
  ops.push_back(std::make_pair(vAST::UnOp::NOR, "~|"));
  ops.push_back(std::make_pair(vAST::UnOp::XOR, "^"));
  ops.push_back(std::make_pair(vAST::UnOp::NXOR, "~^"));
  ops.push_back(std::make_pair(vAST::UnOp::XNOR, "^~"));
  ops.push_back(std::make_pair(vAST::UnOp::PLUS, "+"));
  ops.push_back(std::make_pair(vAST::UnOp::MINUS, "-"));
  vAST::Identifier x("x");
  for (auto it : ops) {
    vAST::UnOp::UnOp op = it.first;
    std::string op_str = it.second;
    vAST::UnaryOp un_op(&x, op);
    EXPECT_EQ(un_op.toString(), op_str + " x");
  }
}

TEST(BasicTests, TestTernaryOp) {
  vAST::Identifier x("x");
  vAST::UnaryOp un_op(&x, vAST::UnOp::INVERT);
  vAST::NumericLiteral zero("0");
  vAST::NumericLiteral one("1");
  vAST::TernaryOp tern_op(&un_op, &one, &zero);
  EXPECT_EQ(tern_op.toString(), "~ x ? 32'd1 : 32'd0");
}

TEST(BasicTests, TestNegEdge) {
  vAST::Identifier clk("clk");
  vAST::NegEdge neg_edge(&clk);

  EXPECT_EQ(neg_edge.toString(), "negedge clk");
}

TEST(BasicTests, TestPosEdge) {
  vAST::Identifier clk("clk");
  vAST::PosEdge pos_edge(&clk);

  EXPECT_EQ(pos_edge.toString(), "posedge clk");
}

TEST(BasicTests, TestPort) {
  vAST::Identifier i("i");
  vAST::Port i_port(&i, vAST::INPUT, vAST::WIRE);

  EXPECT_EQ(i_port.toString(), "input i");

  vAST::Identifier o("o");
  vAST::Port o_port(&o, vAST::OUTPUT, vAST::WIRE);

  EXPECT_EQ(o_port.toString(), "output o");

  vAST::Identifier io("io");
  vAST::Port io_port(&io, vAST::INOUT, vAST::WIRE);

  EXPECT_EQ(io_port.toString(), "inout io");

  vAST::Identifier o_reg("o");
  vAST::Port o_reg_port(&o_reg, vAST::OUTPUT, vAST::REG);

  EXPECT_EQ(o_reg_port.toString(), "output reg o");
}

TEST(BasicTests, TestStringPort) {
  vAST::StringPort port("output reg [width-1:0] I");

  EXPECT_EQ(port.toString(), "output reg [width-1:0] I");
}

TEST(BasicTests, TestModuleInst) {
  std::string module_name = "test_module";

  vAST::NumericLiteral zero("0");
  vAST::NumericLiteral one("1");

  vAST::Identifier param0("param0");
  vAST::Identifier param1("param1");
  vAST::Parameters parameters = {{&param0, &zero}, {&param1, &one}};

  std::string instance_name = "test_module_inst";
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

  vAST::ModuleInstantiation module_inst(module_name, parameters, instance_name,
                                        connections);

  EXPECT_EQ(module_inst.toString(),
            "test_module #(.param0(32'd0), .param1(32'd1)) "
            "test_module_inst(.a(a), .b(b[32'd0]), .c(c[32'd31:32'd0]));");
}

TEST(BasicTests, TestModule) {
  std::string name = "test_module";

  vAST::Identifier i("i");
  vAST::Port i_port(&i, vAST::INPUT, vAST::WIRE);

  vAST::Identifier o("o");
  vAST::Port o_port(&o, vAST::OUTPUT, vAST::WIRE);

  std::vector<vAST::AbstractPort *> ports = {&i_port, &o_port};

  std::vector<std::variant<vAST::StructuralStatement *, vAST::Declaration *>>
      body;

  std::string module_name = "other_module";

  vAST::NumericLiteral zero("0");
  vAST::NumericLiteral one("1");
  vAST::Identifier param0("param0");
  vAST::Identifier param1("param1");

  vAST::Parameters inst_parameters = {{&param0, &zero}, {&param1, &one}};

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

  vAST::Parameters parameters;
  vAST::Module module(name, ports, body, parameters);

  std::string expected_str =
      "module test_module (input i, output o);\nother_module #(.param0(32'd0), "
      ".param1(32'd1)) other_module_inst(.a(a), .b(b[32'd0]), "
      ".c(c[32'd31:32'd0]));\nendmodule\n";
  EXPECT_EQ(module.toString(), expected_str);

  parameters = {{&param0, &zero}, {&param1, &one}};
  vAST::Module module_with_params(name, ports, body, parameters);

  expected_str =
      "module test_module #(parameter param0 = 32'd0, parameter param1 = "
      "32'd1) (input i, output o);\nother_module #(.param0(32'd0), "
      ".param1(32'd1)) other_module_inst(.a(a), .b(b[32'd0]), "
      ".c(c[32'd31:32'd0]));\nendmodule\n";
  EXPECT_EQ(module_with_params.toString(), expected_str);

  std::string string_body = "reg d;\nassign d = a + b;\nassign c = d;";
  vAST::StringBodyModule string_body_module(name, ports, string_body, parameters);
  expected_str =
      "module test_module #(parameter param0 = 32'd0, parameter param1 = "
      "32'd1) (input i, output o);\nreg d;\nassign d = a + b;\nassign c = "
      "d;\nendmodule\n";
  EXPECT_EQ(string_body_module.toString(), expected_str);

  vAST::StringModule string_module(expected_str);
  EXPECT_EQ(string_module.toString(), expected_str);
}

TEST(BasicTests, TestDeclaration) {
  vAST::Identifier a("a");
  vAST::Wire wire(&a);
  EXPECT_EQ(wire.toString(), "wire a;");

  vAST::Reg reg(&a);
  EXPECT_EQ(reg.toString(), "reg a;");

  vAST::Identifier id("x");
  vAST::NumericLiteral high("31");
  vAST::NumericLiteral low("0");
  vAST::Vector slice(&id, &high, &low);
  vAST::Reg reg_slice(&slice);
  EXPECT_EQ(reg_slice.toString(), "reg [32'd31:32'd0] x;");
}

TEST(BasicTests, TestAssign) {
  vAST::Identifier a("a");
  vAST::Identifier b("b");
  vAST::ContinuousAssign cont_assign(&a, &b);
  EXPECT_EQ(cont_assign.toString(), "assign a = b;");

  vAST::BlockingAssign blocking_assign(&a, &b);
  EXPECT_EQ(blocking_assign.toString(), "a = b;");

  vAST::NonBlockingAssign non_blocking_assign(&a, &b);
  EXPECT_EQ(non_blocking_assign.toString(), "a <= b;");
}

TEST(BasicTests, TestAlways) {
  vAST::Identifier a("a");
  vAST::Identifier b("b");
  vAST::Identifier c("c");
  std::vector<std::variant<vAST::Identifier *, vAST::PosEdge *, vAST::NegEdge *,
                           vAST::Star *>>
      sensitivity_list;
  sensitivity_list.push_back(&a);
  vAST::PosEdge posedge(&b);
  sensitivity_list.push_back(&posedge);
  vAST::NegEdge negedge(&c);
  sensitivity_list.push_back(&negedge);
  std::vector<std::variant<vAST::BehavioralStatement *, vAST::Declaration *>>
      body;
  vAST::BlockingAssign assign0(&a, &b);
  body.push_back(&assign0);
  vAST::NonBlockingAssign assign1(&b, &c);
  body.push_back(&assign1);
  vAST::Always always(sensitivity_list, body);
  std::string expected_str =
      "always @(a, posedge b, negedge c) begin\n"
      "a = b;\n"
      "b <= c;\n"
      "end\n";
  EXPECT_EQ(always.toString(), expected_str);

  sensitivity_list.clear();
  vAST::Star star;
  sensitivity_list.push_back(&star);
  vAST::Always always_star(sensitivity_list, body);
  expected_str =
      "always @(*) begin\n"
      "a = b;\n"
      "b <= c;\n"
      "end\n";
  EXPECT_EQ(always_star.toString(), expected_str);

  sensitivity_list.clear();
  ASSERT_THROW(vAST::Always always_empty(sensitivity_list, body),
               std::runtime_error);
}

TEST(BasicTests, File) {
  vAST::Identifier i("i");
  vAST::Port i_port(&i, vAST::INPUT, vAST::WIRE);

  vAST::Identifier o("o");
  vAST::Port o_port(&o, vAST::OUTPUT, vAST::WIRE);

  std::vector<vAST::AbstractPort *> ports = {&i_port, &o_port};

  std::vector<std::variant<vAST::StructuralStatement *, vAST::Declaration *>>
      body;

  std::string module_name = "other_module";

  vAST::NumericLiteral zero("0");
  vAST::NumericLiteral one("1");
  vAST::Identifier param0("param0");
  vAST::Identifier param1("param1");

  vAST::Parameters inst_parameters = {{&param0, &zero}, {&param1, &one}};

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

  vAST::Parameters parameters;
  vAST::Module module("test_module0", ports, body, parameters);
  parameters = {{&param0, &zero}, {&param1, &one}};
  vAST::Module module_with_params("test_module1", ports, body, parameters);
  std::vector<vAST::AbstractModule *> modules;
  modules.push_back(&module);
  modules.push_back(&module_with_params);
  vAST::File file(modules);

  std::string expected_str =
      "module test_module0 (input i, output o);\nother_module "
      "#(.param0(32'd0), .param1(32'd1)) other_module_inst(.a(a), "
      ".b(b[32'd0]), .c(c[32'd31:32'd0]));\nendmodule\n\n"
      "module test_module1 #(parameter param0 = 32'd0, parameter param1 = "
      "32'd1) (input i, output o);\nother_module #(.param0(32'd0), "
      ".param1(32'd1)) other_module_inst(.a(a), .b(b[32'd0]), "
      ".c(c[32'd31:32'd0]));\nendmodule\n";
  EXPECT_EQ(file.toString(), expected_str);
}
TEST(BasicTests, Comment) {
    vAST::SingleLineComment single_line_comment("Test comment");
    EXPECT_EQ(single_line_comment.toString(), "// Test comment");
    vAST::BlockComment block_comment("Test comment\non multiple lines");
    EXPECT_EQ(block_comment.toString(), "/*\nTest comment\non multiple lines\n*/");
}
    

}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
