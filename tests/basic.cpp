#include "verilogAST.hpp"
#include "gtest/gtest.h"

namespace vAST = verilogAST;

namespace {

TEST(BasicTests, TestIdentifier) {
  vAST::Identifier *id = new vAST::Identifier("x");
  ASSERT_EQ(id->toString(), "x");
}

} // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
