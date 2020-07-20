#ifndef VERILOGAST_CONCAT_COALESCER_H
#define VERILOGAST_CONCAT_COALESCER_H

#include "verilogAST.hpp"
#include "verilogAST/transformer.hpp"

namespace verilogAST {

class ConcatCoalescer : public Transformer {
 public:
  using Transformer::visit;

  std::unique_ptr<Expression> visit(std::unique_ptr<Expression> node) override;
};

}  // namespace verilogAST

#endif  // VERILOGAST_CONCAT_COALESCER_H
