#ifndef VERILOGAST_ZEXT_COALESCER_H
#define VERILOGAST_ZEXT_COALESCER_H

#include "verilogAST.hpp"
#include "verilogAST/transformer.hpp"

namespace verilogAST {

class ZextCoalescer : public Transformer {
 public:
  using Transformer::visit;

  std::unique_ptr<Expression> visit(std::unique_ptr<Expression> node) override;
};

}  // namespace verilogAST

#endif  // VERILOGAST_ZEXT_COALESCER_H
