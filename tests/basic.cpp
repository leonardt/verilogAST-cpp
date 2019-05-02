#include "verilogAST.hpp"
#include "gtest/gtest.h"

namespace vAST = verilogAST;

namespace {

TEST(BasicTests, TestNumericLiteral) {
  vAST::NumericLiteral *n = new vAST::NumericLiteral("23", 16, false, "d");
  ASSERT_EQ(n->toString(), "16'd23");
}

TEST(BasicTests, TestIdentifier) {
  vAST::Identifier *id = new vAST::Identifier("x");
  ASSERT_EQ(id->toString(), "x");
}

// TEST(BasicTests, TestIndex) {
//   vAST::Identifier *id = new vAST::Identifier("x");
//   vAST::Index *index = new vAST::Index(id);
//   ASSERT_EQ(id->toString(), "x");
// }

} // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
