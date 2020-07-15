#include "verilogAST/zext_coalescer.hpp"
#include <cassert>

namespace verilogAST {

namespace {

using ConcatArg = std::unique_ptr<Expression>;
using ConcatArgs = std::vector<ConcatArg>;

std::pair<bool, int> num_zeros(Expression* const expr) {
  auto ptr = dynamic_cast<const NumericLiteral*>(expr);
  if (not ptr) return {false, 0};
  switch (ptr->radix) {
    // TODO(rsetaluri): This is conservative in that it will only perform the
    // logic for decimal zero's. This should be expanded to support binary, hex,
    // etc. zero's as well.
    case DECIMAL: {
      auto value = std::atoi(ptr->value.c_str());
      if (value == 0) return {true, ptr->size};
      return {false, 0};
      break;
    }
    default: {
      return {false, 0};
    }
  }
}

std::pair<int, ConcatArgs::const_iterator> processArguments(
    const ConcatArgs& args) {
  int zeros = 0;
  auto it = args.begin();
  for (; it != args.end(); it++) {
    auto res = num_zeros(it->get());
    if (not res.first) break;
    zeros += res.second;
  }
  return {zeros, it};
};

}  // namespace

std::unique_ptr<Expression> ZextCoalescer::visit(
    std::unique_ptr<Expression> node) {
  auto ptr = dynamic_cast<const Concat*>(node.get());
  // This pass only operates on non-empty Concat nodes.
  if (not ptr or ptr->args.size() == 0) return node;
  auto res = processArguments(ptr->args);
  if (res.first == 0) {
    assert(res.second == ptr->args.begin());
    return node;
  }
  ConcatArgs args;
  if (not elide_) {
    args.emplace_back(new NumericLiteral("0", res.first));
  }
  for (auto it = res.second; it != ptr->args.end(); it++) {
    args.push_back((*it)->clone());
  }
  return std::make_unique<Concat>(std::move(args));
}

}  // namespace verilogAST
