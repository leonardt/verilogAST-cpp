#ifndef VERILOGAST_MAKE_PACKED_H
#define VERILOGAST_MAKE_PACKED_H

#include "verilogAST.hpp"
#include "verilogAST/transformer.hpp"

namespace verilogAST {

class MakePacked : public Transformer {
 public:
  MakePacked() = default;

  using Transformer::visit;

  std::unique_ptr<Vector> visit(std::unique_ptr<Vector> vector) override;
};

}  // namespace verilogAST

#endif  // VERILOGAST_MAKE_PACKED_H
