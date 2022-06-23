#include "verilogAST/make_packed.hpp"
#include <cassert>

namespace verilogAST {

std::unique_ptr<Vector> MakePacked::visit(std::unique_ptr<Vector> vector) {
  auto ptr = dynamic_cast<const NDVector*>(vector.get());
  if (not ptr) return vector;
  std::vector<
      std::pair<std::unique_ptr<Expression>, std::unique_ptr<Expression>>>
      outer_dims;
  for (const auto& dim : ptr->outer_dims) {
    outer_dims.push_back(
        std::make_pair(dim.first->clone(), dim.second->clone()));
  }
  return std::make_unique<PackedNDVector>(ptr->id->clone(), ptr->msb->clone(),
                                          ptr->lsb->clone(),
                                          std::move(outer_dims));
}

}  // namespace verilogAST
