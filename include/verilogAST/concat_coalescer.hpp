#ifndef VERILOGAST_ASSIGN_INLINER_H
#define VERILOGAST_ASSIGN_INLINER_H

#include "verilogAST.hpp"
#include "verilogAST/transformer.hpp"

namespace verilogAST {

class ConcatCoalescer : public Transformer {
 public:
  using Transformer::visit;

  std::unique_ptr<Expression> visit(std::unique_ptr<Expression> node) override;
};

}  // namespace verilogAST

#endif  // VERILOGAST_ASSIGN_INLINER_H
