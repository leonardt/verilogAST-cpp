#include "verilogAST.hpp"
#include "gtest/gtest.h"

namespace vAST = verilogAST;

namespace {

TEST(BasicTests, TestNumericLiteral) {
  vAST::NumericLiteral *n =
      new vAST::NumericLiteral("23", 16, false, vAST::DECIMAL);
  EXPECT_EQ(n->toString(), "16'd23");

  n = new vAST::NumericLiteral("DEADBEEF", 32, false, vAST::HEX);
  EXPECT_EQ(n->toString(), "32'hDEADBEEF");

  n = new vAST::NumericLiteral("011001", 6, false, vAST::BINARY);
  EXPECT_EQ(n->toString(), "6'b011001");

  n = new vAST::NumericLiteral("764", 24, false, vAST::OCTAL);
  EXPECT_EQ(n->toString(), "24'o764");
}

TEST(BasicTests, TestIdentifier) {
  vAST::Identifier *id = new vAST::Identifier("x");
  EXPECT_EQ(id->toString(), "x");
}

TEST(BasicTests, TestIndex) {
  vAST::Identifier *id = new vAST::Identifier("x");
  vAST::NumericLiteral *n = new vAST::NumericLiteral("0");
  vAST::Index *index = new vAST::Index(id, n);
  EXPECT_EQ(index->toString(), "x[32'd0]");
}

TEST(BasicTests, TestSlice) {
  vAST::Identifier *id = new vAST::Identifier("x");
  vAST::NumericLiteral *high = new vAST::NumericLiteral("31");
  vAST::NumericLiteral *low = new vAST::NumericLiteral("0");
  vAST::Slice *slice = new vAST::Slice(id, high, low);
  EXPECT_EQ(slice->toString(), "x[32'd31:32'd0]");
}

TEST(BasicTests, TestBinaryOp) {
  // TODO Parametrize test by op
  vAST::Identifier *x = new vAST::Identifier("x");
  vAST::Identifier *y = new vAST::Identifier("y");
  vAST::BinaryOp *bin_op = new vAST::BinaryOp(x, vAST::LSHIFT, y);
  EXPECT_EQ(bin_op->toString(), "x << y");
}

} // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
