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

class FileTransformer : public vAST::Transformer {
 public:
  using vAST::Transformer::visit;
  virtual std::unique_ptr<vAST::Identifier> visit(
      std::unique_ptr<vAST::Identifier> node) {
    if (node->value == "o") {
      return vAST::make_id("d");
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
  std::vector<std::unique_ptr<vAST::Expression>> call_args;
  call_args.push_back(std::make_unique<vAST::TernaryOp>(
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
                                        vAST::make_id("x"))));
  call_args.push_back(vAST::make_id("x"));
  std::unique_ptr<vAST::Expression> expr =
      std::make_unique<vAST::CallExpr>("my_func", std::move(call_args));
  XtoZ transformer;
  EXPECT_EQ(transformer.visit(std::move(expr))->toString(),
            "my_func({z,b} ? z[3:1] + (~ y[1]) : {(2){z}}, z)");
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
  std::vector<std::unique_ptr<vAST::AbstractPort>> ports;
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("i"), vAST::INPUT,
                                               vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(
      std::make_unique<vAST::Vector>(vAST::make_id("o"), vAST::make_num("3"),
                                     vAST::make_id("c")),
      vAST::OUTPUT, vAST::WIRE));

  ports.push_back(
      std::make_unique<vAST::StringPort>("output reg [width-1:0] k"));

  std::vector<std::variant<std::unique_ptr<vAST::StructuralStatement>,
                           std::unique_ptr<vAST::Declaration>>>
      body = make_simple_body();

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("c"),
      std::make_unique<vAST::Identifier>("b")));

  body.push_back(
      std::make_unique<vAST::Wire>(std::make_unique<vAST::Identifier>("c")));

  body.push_back(
      std::make_unique<vAST::Reg>(std::make_unique<vAST::Identifier>("c")));

  std::vector<std::variant<
      std::unique_ptr<vAST::Identifier>, std::unique_ptr<vAST::PosEdge>,
      std::unique_ptr<vAST::NegEdge>, std::unique_ptr<vAST::Star>>>
      sensitivity_list;
  sensitivity_list.push_back(std::make_unique<vAST::Identifier>("a"));

  body.push_back(std::make_unique<vAST::Always>(std::move(sensitivity_list),
                                                make_simple_always_body()));


  body.push_back(std::make_unique<vAST::SingleLineComment>("Test comment"));
  body.push_back(std::make_unique<vAST::BlockComment>("Test comment\non multiple lines"));
  std::unique_ptr<vAST::AbstractModule> module = std::make_unique<vAST::Module>(
      "test_module0", std::move(ports), std::move(body), make_simple_params());
  std::string expected_str =
      "module test_module0 #(parameter y = 0, parameter param1 = "
      "1) (input i, output [3:g] o, output reg [width-1:0] k);\nother_module "
      "#(.y(0), .param1(1)) other_module_inst(.a(a), .b(b[0]), "
      ".c(g[31:0]));\nassign g = b;\nwire g;\nreg g;\n"
      "always @(a) begin\n"
      "a = b;\n"
      "b <= g;\n"
      "$display(\"b=%d, c=%d\", b, g);\n"
      "end\n\n"
      "// Test comment\n"
      "/*\nTest comment\non multiple lines\n*/\n"
      "endmodule\n";

  ModuleTransformer transformer;
  EXPECT_EQ(transformer.visit(std::move(module))->toString(), expected_str);
}
TEST(TransformerTests, File) {
  std::vector<std::unique_ptr<vAST::AbstractModule>> modules;
  vAST::Parameters parameters0;
  std::string name = "test_module";

  std::string module_name = "other_module";

  std::string string_body = "reg d;\nassign d = a + b;\nassign c = d;";
  modules.push_back(std::make_unique<vAST::StringBodyModule>(
      name, make_simple_ports(), string_body, make_simple_params()));

  std::string module_str =
      "module string_module #(parameter param0 = 0, parameter param1 = "
      "1) (input i, output o);\nreg d;\nassign d = a + b;\nassign c = "
      "d;\nendmodule\n";
  modules.push_back(std::make_unique<vAST::StringModule>(module_str));

  std::unique_ptr<vAST::File> file = std::make_unique<vAST::File>(modules);

  std::string expected_str =
      "module test_module #(parameter y = 0, parameter param1 = "
      "1) (input i, output d);\nreg d;\nassign d = a + b;\nassign c = "
      "d;\nendmodule\n\n"
      "module string_module #(parameter param0 = 0, parameter param1 = "
      "1) (input i, output o);\nreg d;\nassign d = a + b;\nassign c = "
      "d;\nendmodule\n";
  FileTransformer transformer;
  EXPECT_EQ(transformer.visit(std::move(file))->toString(), expected_str);
}
}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
