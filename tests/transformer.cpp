#include "common.cpp"
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

class AlwaysTransformer : public vAST::Transformer {
 public:
  using vAST::Transformer::visit;
  virtual std::unique_ptr<vAST::Identifier> visit(
      std::unique_ptr<vAST::Identifier> node) {
    if (node->value == "a") {
      return vAST::make_id("z");
    } else if (node->value == "b") {
      return vAST::make_id("y");
    }
    return node;
  };
};

class ModuleTransformer : public vAST::Transformer {
 public:
  using vAST::Transformer::visit;
  virtual std::unique_ptr<vAST::Identifier> visit(
      std::unique_ptr<vAST::Identifier> node) {
    if (node->value == "c") {
      return vAST::make_id("g");
    } else if (node->value == "param0") {
      return vAST::make_id("y");
    }
    return node;
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
          std::make_unique<vAST::UnaryOp>(
              std::make_unique<vAST::Index>(vAST::make_id("y"),
                                            vAST::make_num("1")),
              vAST::UnOp::INVERT)),
      std::make_unique<vAST::Replicate>(vAST::make_num("2"),
                                        vAST::make_id("x")));
  XtoZ transformer;
  EXPECT_EQ(transformer.visit(std::move(expr))->toString(),
            "{z,b} ? z[3:1] + (~ y[1]) : {(2){z}}");
}
TEST(TransformerTests, TestReplaceNameWithExpr) {
  std::unique_ptr<vAST::Expression> expr = vAST::make_binop(
      vAST::make_id("x"), vAST::BinOp::MUL, vAST::make_id("y"));
  ReplaceNameWithExpr transformer;
  EXPECT_EQ(transformer.visit(std::move(expr))->toString(), "(z - w) * y");
}
TEST(TransformerTests, TestAlways) {
  std::vector<std::variant<
      std::unique_ptr<vAST::Identifier>, std::unique_ptr<vAST::PosEdge>,
      std::unique_ptr<vAST::NegEdge>, std::unique_ptr<vAST::Star>>>
      sensitivity_list;
  sensitivity_list.push_back(std::make_unique<vAST::Identifier>("a"));
  sensitivity_list.push_back(
      std::make_unique<vAST::PosEdge>(std::make_unique<vAST::Identifier>("b")));
  sensitivity_list.push_back(
      std::make_unique<vAST::NegEdge>(std::make_unique<vAST::Identifier>("c")));
  sensitivity_list.push_back(std::make_unique<vAST::Star>());
  std::unique_ptr<vAST::Always> always = std::make_unique<vAST::Always>(
      std::move(sensitivity_list), make_simple_always_body());
  std::string expected_str =
      "always @(z, posedge y, negedge c, *) begin\n"
      "z = y;\n"
      "y <= c;\n"
      "$display(\"b=%d, c=%d\", y, c);\n"
      "end\n";
  AlwaysTransformer transformer;
  EXPECT_EQ(transformer.visit(std::move(always))->toString(), expected_str);
}
TEST(TransformerTests, TestModule) {
  std::unique_ptr<vAST::AbstractModule> module =
      std::make_unique<vAST::Module>("test_module0", make_simple_ports(),
                                     make_simple_body(), make_simple_params());
  std::string expected_str =
      "module test_module0 #(parameter y = 0, parameter param1 = "
      "1) (input i, output o);\nother_module #(.y(0), "
      ".param1(1)) other_module_inst(.a(a), .b(b[0]), "
      ".c(g[31:0]));\nendmodule\n";

  ModuleTransformer transformer;
  EXPECT_EQ(transformer.visit(std::move(module))->toString(), expected_str);
}
}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
