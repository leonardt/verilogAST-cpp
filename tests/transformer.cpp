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
  std::vector<std::unique_ptr<vAST::Expression>> concat_args;
  concat_args.push_back(vAST::make_id("x"));
  concat_args.push_back(vAST::make_id("b"));
  std::unique_ptr<vAST::Expression> expr = std::make_unique<vAST::TernaryOp>(
      std::make_unique<vAST::Concat>(std::move(concat_args)),
      vAST::make_binop(
          std::make_unique<vAST::Slice>(vAST::make_id("x"), vAST::make_num("3"),
                                        vAST::make_num("1")),
          vAST::BinOp::ADD,
          std::make_unique<vAST::UnaryOp>(vAST::make_id("y"),
                                          vAST::UnOp::INVERT)),
      std::make_unique<vAST::Replicate>(vAST::make_num("2"),
                                        vAST::make_id("x")));
  XtoZ transformer;
  EXPECT_EQ(transformer.visit(std::move(expr))->toString(), "{z,b} ? z[3:1] + (~ y) : {(2){z}}");
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
