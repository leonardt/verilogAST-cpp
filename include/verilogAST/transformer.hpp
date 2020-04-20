#pragma once
#ifndef VERILOGAST_TRANSFORMER_H
#define VERILOGAST_TRANSFORMER_H
#include "verilogAST.hpp"

namespace verilogAST {

class Transformer {
 public:
  template <typename T>
  T visit(T node) {
    return std::visit(
        [&](auto&& value) -> T { return this->visit(std::move(value)); }, node);
  }

  virtual std::unique_ptr<Expression> visit(std::unique_ptr<Expression> node);

  virtual std::unique_ptr<NumericLiteral> visit(
      std::unique_ptr<NumericLiteral> node);

  virtual std::unique_ptr<Identifier> visit(std::unique_ptr<Identifier> node);

  virtual std::unique_ptr<Cast> visit(std::unique_ptr<Cast> node);

  virtual std::unique_ptr<Attribute> visit(std::unique_ptr<Attribute> node);

  virtual std::unique_ptr<String> visit(std::unique_ptr<String> node);

  virtual std::unique_ptr<Index> visit(std::unique_ptr<Index> node);

  virtual std::unique_ptr<Slice> visit(std::unique_ptr<Slice> node);

  virtual std::unique_ptr<BinaryOp> visit(std::unique_ptr<BinaryOp> node);

  virtual std::unique_ptr<UnaryOp> visit(std::unique_ptr<UnaryOp> node);

  virtual std::unique_ptr<TernaryOp> visit(std::unique_ptr<TernaryOp> node);

  virtual std::unique_ptr<Concat> visit(std::unique_ptr<Concat> node);

  virtual std::unique_ptr<Replicate> visit(std::unique_ptr<Replicate> node);

  virtual std::unique_ptr<NegEdge> visit(std::unique_ptr<NegEdge> node);

  virtual std::unique_ptr<PosEdge> visit(std::unique_ptr<PosEdge> node);

  virtual std::unique_ptr<CallExpr> visit(std::unique_ptr<CallExpr> node);

  virtual std::variant<std::unique_ptr<Identifier>, std::unique_ptr<Vector>>
  visit(
      std::variant<std::unique_ptr<Identifier>, std::unique_ptr<Vector>> node);

  virtual std::unique_ptr<Vector> visit(std::unique_ptr<Vector> node);

  virtual std::unique_ptr<Port> visit(std::unique_ptr<Port> node);

  virtual std::unique_ptr<StringPort> visit(std::unique_ptr<StringPort> node);

  virtual std::unique_ptr<SingleLineComment> visit(
      std::unique_ptr<SingleLineComment> node);

  virtual std::unique_ptr<BlockComment> visit(
      std::unique_ptr<BlockComment> node);

  virtual std::unique_ptr<InlineVerilog> visit(
      std::unique_ptr<InlineVerilog> node);

  virtual std::unique_ptr<ModuleInstantiation> visit(
      std::unique_ptr<ModuleInstantiation> node);

  virtual std::unique_ptr<Wire> visit(std::unique_ptr<Wire> node);

  virtual std::unique_ptr<Reg> visit(std::unique_ptr<Reg> node);

  virtual std::unique_ptr<ContinuousAssign> visit(
      std::unique_ptr<ContinuousAssign> node);

  virtual std::unique_ptr<Declaration> visit(std::unique_ptr<Declaration> node);

  virtual std::unique_ptr<BehavioralStatement> visit(
      std::unique_ptr<BehavioralStatement> node);

  virtual std::unique_ptr<BlockingAssign> visit(
      std::unique_ptr<BlockingAssign> node);

  virtual std::unique_ptr<NonBlockingAssign> visit(
      std::unique_ptr<NonBlockingAssign> node);

  virtual std::unique_ptr<CallStmt> visit(std::unique_ptr<CallStmt> node);

  virtual std::unique_ptr<Star> visit(std::unique_ptr<Star> node);

  virtual std::unique_ptr<Always> visit(std::unique_ptr<Always> node);

  virtual std::unique_ptr<AbstractPort> visit(
      std::unique_ptr<AbstractPort> node);

  virtual std::unique_ptr<StructuralStatement> visit(
      std::unique_ptr<StructuralStatement> node);

  virtual std::unique_ptr<Module> visit(std::unique_ptr<Module> node);

  virtual std::unique_ptr<StringBodyModule> visit(
      std::unique_ptr<StringBodyModule> node);

  virtual std::unique_ptr<StringModule> visit(
      std::unique_ptr<StringModule> node);

  virtual std::unique_ptr<AbstractModule> visit(
      std::unique_ptr<AbstractModule> node);

  virtual std::unique_ptr<File> visit(std::unique_ptr<File> node);
};

}  // namespace verilogAST
#endif
