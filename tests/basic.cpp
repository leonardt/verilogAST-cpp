#include "verilogAST.hpp"
#include "gtest/gtest.h"

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
}

TEST(BasicTests, TestIdentifier) {
  vAST::Identifier id("x");
  EXPECT_EQ(id.toString(), "x");
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

} // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
