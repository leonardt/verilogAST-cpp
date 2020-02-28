#include "verilogAST/concat_coalescer.hpp"

namespace verilogAST {

namespace {

struct Run {
  std::string name;
  int first;  // inclusive
  int last;  // inclusive
};

class RunOrExpr {
 public:
  explicit RunOrExpr(const Expression* expr) : run_(), expr_(expr) {}
  explicit RunOrExpr(std::string name, int first, int last)
      : run_({name, first, last}), expr_(nullptr) {}

  bool isRun() const { return expr_ == nullptr; }

  // Tries to merge in @other into this run if it is contiguous. Returns true if
  // merged, falses otherwise.
  bool tryMerge(const RunOrExpr& other) {
    if (!isRun() or !other.isRun()) return false;
    if (run_.name != other.run_.name) return false;
    if (run_.last != other.run_.first + 1) return false;
    run_.last = other.run_.first;
    return true;
  }

  // Returns the expression corresponding to this run. If this is not a run,
  // then we return a clone of the contained expression. Otherwise, we return an
  // Index expresssion if the run contains only one index, or a Slice if it
  // contains > 1.
  std::unique_ptr<Expression> generateExpression() const {
    if (not isRun()) return expr_->clone();
    auto first = std::unique_ptr<Expression>(
        new NumericLiteral(std::to_string(run_.first)));
    if (run_.first == run_.last) {
      return std::unique_ptr<Expression>(
          new Index(std::make_unique<Identifier>(run_.name), std::move(first)));
    }
    auto id = std::unique_ptr<Expression>(new Identifier(run_.name));
    auto last = std::unique_ptr<Expression>(
        new NumericLiteral(std::to_string(run_.last)));
    return std::unique_ptr<Expression>(
        new Slice(std::move(id), std::move(first), std::move(last)));
  }

 private:
  Run run_;
  const Expression* expr_ = nullptr;
};

// Tries to extract a NumericLiteral (int) from @expr. Returns a pair of <true,
// value> if successful; otherwise, returns <false, 0>.
std::pair<bool, int> expr_to_int(const Expression* expr) {
  auto ptr = dynamic_cast<const NumericLiteral*>(expr);
  if (not ptr) return std::make_pair(false, 0);
  return std::make_pair(true, std::atoi(ptr->value.c_str()));
}

// Consumes a Concat node argument @arg and tries to make a run out of it. If it
// is of the form Index(Identifier name, NumericLiteral val) then we return
// RunOrExpr(name, val, val); otherwise we return RunOrExpr(arg).
RunOrExpr makeRunOrExpr(const Expression* arg) {
  auto index = dynamic_cast<const Index*>(arg);
  if (not index) return RunOrExpr(arg);
  auto as_int = expr_to_int(index->index.get());
  if (not as_int.first) return RunOrExpr(arg);
  return RunOrExpr(index->id->value, as_int.second, as_int.second);
}

}  // namespace

std::unique_ptr<Expression> ConcatCoalescer::visit(
    std::unique_ptr<Expression> node) {
  auto ptr = dynamic_cast<const Concat*>(node.get());
  // This pass only operates on non-empty Concat nodes.
  if (not ptr or ptr->args.size() == 0) return node;
  std::vector<RunOrExpr> runs;
  for (const auto& arg : ptr->args) {
    auto run_or_expr = makeRunOrExpr(arg.get());
    // If this is the first run, then we append it. Otherwise, we try to merge
    // it into the previous run. If it cannot be merged, then we append it.
    if (runs.size() == 0 or not runs.back().tryMerge(run_or_expr)) {
      runs.push_back(run_or_expr);
    }
  }
  assert(runs.size() > 0);
  // If there is sonly one run, then we return that run as a standalone
  // expression; otherwise, we return a Concat node containing the runs.
  if (runs.size() == 1) return runs.front().generateExpression();
  std::vector<std::unique_ptr<Expression>> args;
  for (const auto& run : runs) {
    args.push_back(run.generateExpression());
  }
  return std::make_unique<Concat>(std::move(args));
}

}  // namespace verilogAST
