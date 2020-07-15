#include "verilogAST/zext_coalescer.hpp"
#include "common.cpp"
#include "gtest/gtest.h"

namespace vAST = verilogAST;

namespace {

using BodyElement = std::variant<std::unique_ptr<vAST::StructuralStatement>,
                                 std::unique_ptr<vAST::Declaration>>;

auto makeVector(std::string name, int hi, int lo) {
  return std::make_unique<vAST::Vector>(vAST::make_id(name),
                                        vAST::make_num(std::to_string(hi)),
                                        vAST::make_num(std::to_string(lo)));
}

void runTest(std::vector<int> prefix,
             std::string pre,
             std::string post,
             bool elide = false) {
  std::vector<std::unique_ptr<vAST::AbstractPort>> ports;
  ports.push_back(std::make_unique<vAST::Port>(makeVector("I", 7, 0),
                                               vAST::INPUT,
                                               vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(makeVector("O", 7, 0),
                                               vAST::OUTPUT,
                                               vAST::WIRE));
  std::vector<BodyElement> body;

  std::vector<std::unique_ptr<vAST::Expression>> args = {};
  int remaining_size = 8;
  for (const auto& it : prefix) {
    args.emplace_back(new vAST::NumericLiteral("0", it));
    remaining_size -= it;
  }
  args.emplace_back(
      new vAST::Slice(
          vAST::make_id("I"),
          vAST::make_num(std::to_string(remaining_size - 1)),
          vAST::make_num("0")));
  std::unique_ptr<vAST::Expression> rhs(new vAST::Concat(std::move(args)));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("O"), std::move(rhs)));

  std::unique_ptr<vAST::AbstractModule> module = std::make_unique<vAST::Module>(
      "test_module",
      std::move(ports),
      std::move(body));

  EXPECT_EQ(module->toString(), pre);

  // Run ZextCoalescer transformer.
  vAST::ZextCoalescer transformer(elide);
  module = transformer.visit(std::move(module));

  EXPECT_EQ(module->toString(), post);
}

TEST(ZextCoalescerTests, TestBasic) {
  auto pre =
      "module test_module (\n"
      "    input [7:0] I,\n"
      "    output [7:0] O\n"
      ");\n"
      "assign O = {1'd0,2'd0,I[4:0]};\n"
      "endmodule\n";
  auto post =
      "module test_module (\n"
      "    input [7:0] I,\n"
      "    output [7:0] O\n"
      ");\n"
      "assign O = {3'd0,I[4:0]};\n"
      "endmodule\n";
  runTest({1, 2}, pre, post);
}

TEST(ZextCoalescerTests, TestElide) {
  auto pre =
      "module test_module (\n"
      "    input [7:0] I,\n"
      "    output [7:0] O\n"
      ");\n"
      "assign O = {1'd0,2'd0,I[4:0]};\n"
      "endmodule\n";
  auto post =
      "module test_module (\n"
      "    input [7:0] I,\n"
      "    output [7:0] O\n"
      ");\n"
      "assign O = {I[4:0]};\n"
      "endmodule\n";
  runTest({1, 2}, pre, post, true /* elide */);
}

// TEST(ZextCoalescerTests, TestMultipleRuns) {
//   auto pre =
//       "module test_module (\n"
//       "    input [7:0] I,\n"
//       "    output [7:0] O\n"
//       ");\n"
//       "assign O = {I[7],I[6],I[5],I[2],I[1],I[0],I[4],I[3]};\n"
//       "endmodule\n";
//   auto post =
//       "module test_module (\n"
//       "    input [7:0] I,\n"
//       "    output [7:0] O\n"
//       ");\n"
//       "assign O = {I[7:5],I[2:0],I[4:3]};\n"
//       "endmodule\n";
//   runTest({3, 4, 0, 1, 2, 5, 6, 7}, pre, post);
// }

// TEST(ZextCoalescerTests, TestNoRuns) {
//   auto pre =
//       "module test_module (\n"
//       "    input [7:0] I,\n"
//       "    output [7:0] O\n"
//       ");\n"
//       "assign O = {I[7],I[5],I[3],I[1],I[6],I[4],I[2],I[0]};\n"
//       "endmodule\n";
//   auto post =
//       "module test_module (\n"
//       "    input [7:0] I,\n"
//       "    output [7:0] O\n"
//       ");\n"
//       "assign O = {I[7],I[5],I[3],I[1],I[6],I[4],I[2],I[0]};\n"
//       "endmodule\n";
//   // Sequence has no contiguous runs.
//   runTest({0, 2, 4, 6, 1, 3, 5, 7}, pre, post);
// }

// TEST(ZextCoalescerTests, ReverseRun) {
//   auto pre =
//       "module test_module (\n"
//       "    input [7:0] I,\n"
//       "    output [7:0] O\n"
//       ");\n"
//       "assign O = {I[4],I[5],I[6],I[7],I[3],I[2],I[1],I[0]};\n"
//       "endmodule\n";
//   // ZextCoalescer doesn't coalese reverse runs right now.
//   auto post =
//       "module test_module (\n"
//       "    input [7:0] I,\n"
//       "    output [7:0] O\n"
//       ");\n"
//       "assign O = {I[4],I[5],I[6],I[7],I[3:0]};\n"
//       "endmodule\n";
//   runTest({0, 1, 2, 3, 7, 6, 5, 4}, pre, post);
// }

// TEST(ZextCoalescerTests, MultipleNames) {
//   auto pre =
//       "module test_module (\n"
//       "    input [7:0] I0,\n"
//       "    input [7:0] I1,\n"
//       "    output [7:0] O\n"
//       ");\n"
//       "assign O = {I0[7],I0[6],I0[5],I0[4],I1[3],I1[2],I1[1],I1[0]};\n"
//       "endmodule\n";
//   auto post =
//       "module test_module (\n"
//       "    input [7:0] I0,\n"
//       "    input [7:0] I1,\n"
//       "    output [7:0] O\n"
//       ");\n"
//       "assign O = {I0[7:4],I1[3:0]};\n"
//       "endmodule\n";
//   std::vector<std::unique_ptr<vAST::AbstractPort>> ports;
//   ports.push_back(std::make_unique<vAST::Port>(makeVector("I0", 7, 0),
//                                                vAST::INPUT,
//                                                vAST::WIRE));
//   ports.push_back(std::make_unique<vAST::Port>(makeVector("I1", 7, 0),
//                                                vAST::INPUT,
//                                                vAST::WIRE));
//   ports.push_back(std::make_unique<vAST::Port>(makeVector("O", 7, 0),
//                                                vAST::OUTPUT,
//                                                vAST::WIRE));
//   std::vector<BodyElement> body;

//   std::vector<std::unique_ptr<vAST::Expression>> args = {};
//   for (int i = 7; i >= 0; i--) {
//     auto id = i > 3 ? "I0" : "I1";
//     args.emplace_back(
//         new vAST::Index(
//             vAST::make_id(id),
//             vAST::make_num(std::to_string(i))));
//   }
//   std::unique_ptr<vAST::Expression> rhs(new vAST::Concat(std::move(args)));

//   body.push_back(std::make_unique<vAST::ContinuousAssign>(
//       std::make_unique<vAST::Identifier>("O"), std::move(rhs)));

//   std::unique_ptr<vAST::AbstractModule> module = std::make_unique<vAST::Module>(
//       "test_module",
//       std::move(ports),
//       std::move(body));

//   EXPECT_EQ(module->toString(), pre);

//   // Run ZextCoalescer transformer.
//   vAST::ZextCoalescer transformer;
//   module = transformer.visit(std::move(module));

//   EXPECT_EQ(module->toString(), post);
// }

}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
