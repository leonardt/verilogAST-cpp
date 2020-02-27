#include "verilogAST/concat_coalescer.hpp"

namespace verilogAST {

namespace {

std::pair<bool, int> get_index_integer(const Expression* expr) {
  auto ptr = dynamic_cast<const NumericLiteral*>(expr);
  if (not ptr) return std::make_pair(false, 0);
  return std::make_pair(true, std::atoi(ptr->value.c_str()));
}

}  // namespace

std::unique_ptr<Expression> ConcatCoalescer::visit(
    std::unique_ptr<Expression> node) {
  auto ptr = dynamic_cast<const Concat*>(node.get());
  if (not ptr) return node;
  const Index* first = nullptr;
  const Index* last = nullptr;
  int last_index = 0;
  for (const auto& arg : ptr->args) {
    auto curr = dynamic_cast<const Index*>(arg.get());
    if (not curr) return node;
    const auto as_int = get_index_integer(curr->index.get());
    if (not as_int.first) return node;
    if (not first) {
      first = curr;
      last = curr;
      last_index = as_int.second;
      continue;
    }
    if (curr->id->value != last->id->value) return node;
    if (as_int.second != last_index - 1) return node;
    last_index = as_int.second;
    last = curr;
  }
  return std::make_unique<Slice>(first->id->clone(),
                                 first->index->clone(),
                                 last->index->clone());
}

}  // namespace verilogAST
