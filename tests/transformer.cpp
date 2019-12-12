#include "gtest/gtest.h"
#include "verilogAST.hpp"

namespace vAST = verilogAST;

class XtoZ : public vAST::Transformer {
 public:
  using vAST::Transformer::visit;
  virtual std::unique_ptr<vAST::Identifier> visit(
      std::unique_ptr<vAST::Identifier> node) {
    if (node->value == "x") {
      return vAST::make_id("z");
    }
    return node;
  };
};

class ReplaceNameWithExpr : public vAST::Transformer {
 public:
  using vAST::Transformer::visit;
  virtual std::unique_ptr<vAST::Expression> visit(
      std::unique_ptr<vAST::Expression> node) {
    if (auto ptr = dynamic_cast<vAST::Identifier *>(node.get())) {
      node.release();
      std::unique_ptr<vAST::Identifier> id(ptr);
      if (id->value == "x") {
        return vAST::make_binop(vAST::make_id("z"), vAST::BinOp::SUB,
                                vAST::make_id("w"));
      }
      return vAST::Transformer::visit(std::move(id));
    }
    return vAST::Transformer::visit(std::move(node));
  };
};

namespace {
TEST(TransformerTests, TestXtoZ) {
  std::unique_ptr<vAST::Expression> expr = vAST::make_binop(
      vAST::make_id("x"), vAST::BinOp::ADD, vAST::make_id("y"));
  XtoZ transformer;
  EXPECT_EQ(transformer.visit(std::move(expr))->toString(), "z + y");
}
TEST(TransformerTests, TestReplaceNameWithExpr) {
  std::unique_ptr<vAST::Expression> expr = vAST::make_binop(
      vAST::make_id("x"), vAST::BinOp::MUL, vAST::make_id("y"));
  ReplaceNameWithExpr transformer;
  EXPECT_EQ(transformer.visit(std::move(expr))->toString(), "(z - w) * y");
}
}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
