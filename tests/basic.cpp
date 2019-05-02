#include "verilogAST.hpp"
#include "gtest/gtest.h"

namespace vAST = verilogAST;

namespace {

TEST(BasicTests, TestNumericLiteral) {
  vAST::NumericLiteral *n = new vAST::NumericLiteral("23", 16, false, vAST::decimal);
  EXPECT_EQ(n->toString(), "16'd23");

  n = new vAST::NumericLiteral("DEADBEEF", 32, false, vAST::hex);
  EXPECT_EQ(n->toString(), "32'hDEADBEEF");

  n = new vAST::NumericLiteral("011001", 6, false, vAST::binary);
  EXPECT_EQ(n->toString(), "6'b011001");

  n = new vAST::NumericLiteral("764", 24, false, vAST::octal);
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

} // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
