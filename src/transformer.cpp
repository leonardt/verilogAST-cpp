#include "verilogAST/transformer.hpp"
#include <iostream>

namespace verilogAST {

std::unique_ptr<Expression> Transformer::visit(
    std::unique_ptr<Expression> node) {
  if (auto ptr = dynamic_cast<NumericLiteral*>(node.get())) {
    node.release();
    return this->visit(std::unique_ptr<NumericLiteral>(ptr));
  }
  if (auto ptr = dynamic_cast<Identifier*>(node.get())) {
    node.release();
    return this->visit(std::unique_ptr<Identifier>(ptr));
  }
  if (auto ptr = dynamic_cast<String*>(node.get())) {
    node.release();
    return this->visit(std::unique_ptr<String>(ptr));
  }
  if (auto ptr = dynamic_cast<Index*>(node.get())) {
    node.release();
    return this->visit(std::unique_ptr<Index>(ptr));
  }
  if (auto ptr = dynamic_cast<Slice*>(node.get())) {
    node.release();
    return this->visit(std::unique_ptr<Slice>(ptr));
  }
  if (auto ptr = dynamic_cast<BinaryOp*>(node.get())) {
    node.release();
    return this->visit(std::unique_ptr<BinaryOp>(ptr));
  }
  if (auto ptr = dynamic_cast<UnaryOp*>(node.get())) {
    node.release();
    return this->visit(std::unique_ptr<UnaryOp>(ptr));
  }
  if (auto ptr = dynamic_cast<TernaryOp*>(node.get())) {
    node.release();
    return this->visit(std::unique_ptr<TernaryOp>(ptr));
  }
  if (auto ptr = dynamic_cast<Concat*>(node.get())) {
    node.release();
    return this->visit(std::unique_ptr<Concat>(ptr));
  }
  if (auto ptr = dynamic_cast<Replicate*>(node.get())) {
    node.release();
    return this->visit(std::unique_ptr<Replicate>(ptr));
  }
  if (auto ptr = dynamic_cast<CallExpr*>(node.get())) {
    node.release();
    return this->visit(std::unique_ptr<CallExpr>(ptr));
  }
  throw std::runtime_error("Unreachable");  // LCOV_EXCL_LINE
  return node;                              // LCOV_EXCL_LINE
}

std::unique_ptr<NumericLiteral> Transformer::visit(
    std::unique_ptr<NumericLiteral> node) {
  return node;
}

std::unique_ptr<Identifier> Transformer::visit(
    std::unique_ptr<Identifier> node) {
  return node;
}

std::unique_ptr<String> Transformer::visit(std::unique_ptr<String> node) {
  return node;
}

std::unique_ptr<Index> Transformer::visit(std::unique_ptr<Index> node) {
  node->id = this->visit(std::move(node->id));
  node->index = this->visit(std::move(node->index));
  return node;
}

std::unique_ptr<Slice> Transformer::visit(std::unique_ptr<Slice> node) {
  node->expr = this->visit(std::move(node->expr));
  node->high_index = this->visit(std::move(node->high_index));
  node->low_index = this->visit(std::move(node->low_index));
  return node;
}

std::unique_ptr<BinaryOp> Transformer::visit(std::unique_ptr<BinaryOp> node) {
  node->left = this->visit(std::move(node->left));
  node->right = this->visit(std::move(node->right));
  return node;
}

std::unique_ptr<UnaryOp> Transformer::visit(std::unique_ptr<UnaryOp> node) {
  node->operand = this->visit(std::move(node->operand));
  return node;
}

std::unique_ptr<TernaryOp> Transformer::visit(std::unique_ptr<TernaryOp> node) {
  node->cond = this->visit(std::move(node->cond));
  node->true_value = this->visit(std::move(node->true_value));
  node->false_value = this->visit(std::move(node->false_value));
  return node;
}

std::unique_ptr<Concat> Transformer::visit(std::unique_ptr<Concat> node) {
  std::vector<std::unique_ptr<Expression>> new_args;
  for (auto&& expr : node->args) {
    new_args.push_back(this->visit(std::move(expr)));
  }
  node->args = std::move(new_args);
  return node;
}

std::unique_ptr<Replicate> Transformer::visit(std::unique_ptr<Replicate> node) {
  node->num = this->visit(std::move(node->num));
  node->value = this->visit(std::move(node->value));
  return node;
}

std::unique_ptr<NegEdge> Transformer::visit(std::unique_ptr<NegEdge> node) {
  node->value = this->visit(std::move(node->value));
  return node;
}

std::unique_ptr<PosEdge> Transformer::visit(std::unique_ptr<PosEdge> node) {
  node->value = this->visit(std::move(node->value));
  return node;
}

std::unique_ptr<CallExpr> Transformer::visit(std::unique_ptr<CallExpr> node) {
  std::vector<std::unique_ptr<Expression>> new_args;
  for (auto&& expr : node->args) {
    new_args.push_back(this->visit(std::move(expr)));
  }
  node->args = std::move(new_args);
  return node;
}

std::variant<std::unique_ptr<Identifier>, std::unique_ptr<Vector>>
Transformer::visit(
    std::variant<std::unique_ptr<Identifier>, std::unique_ptr<Vector>> node) {
  return std::visit(
      [&](auto&& value) -> std::variant<std::unique_ptr<Identifier>,
                                        std::unique_ptr<Vector>> {
        if (auto ptr = dynamic_cast<Identifier*>(value.get())) {
          value.release();
          return this->visit(std::unique_ptr<Identifier>(ptr));
        }
        if (auto ptr = dynamic_cast<Vector*>(value.get())) {
          value.release();
          return this->visit(std::unique_ptr<Vector>(ptr));
        }
        throw std::runtime_error("Unreachable");  // LCOV_EXCL_LINE
        return std::move(value);                  // LCOV_EXCL_LINE
      },
      node);
}

std::unique_ptr<Vector> Transformer::visit(std::unique_ptr<Vector> node) {
  node->id = this->visit(std::move(node->id));
  node->msb = this->visit(std::move(node->msb));
  node->lsb = this->visit(std::move(node->lsb));
  return node;
}

std::unique_ptr<Port> Transformer::visit(std::unique_ptr<Port> node) {
  node->value = this->visit(std::move(node->value));
  return node;
}

std::unique_ptr<StringPort> Transformer::visit(
    std::unique_ptr<StringPort> node) {
  return node;
}

std::unique_ptr<SingleLineComment> Transformer::visit(
    std::unique_ptr<SingleLineComment> node) {
  return node;
}

std::unique_ptr<BlockComment> Transformer::visit(
    std::unique_ptr<BlockComment> node) {
  return node;
}

std::unique_ptr<InlineVerilog> Transformer::visit(
    std::unique_ptr<InlineVerilog> node) {
  return node;
}

std::unique_ptr<ModuleInstantiation> Transformer::visit(
    std::unique_ptr<ModuleInstantiation> node) {
  for (auto&& conn : *node->connections) {
    conn.second = this->visit(std::move(conn.second));
  }
  for (auto&& param : node->parameters) {
    param.first = this->visit(std::move(param.first));
    param.second = this->visit(std::move(param.second));
  }
  return node;
}

std::unique_ptr<Wire> Transformer::visit(std::unique_ptr<Wire> node) {
  node->value = this->visit(std::move(node->value));
  return node;
}

std::unique_ptr<Reg> Transformer::visit(std::unique_ptr<Reg> node) {
  node->value = this->visit(std::move(node->value));
  return node;
}

std::unique_ptr<ContinuousAssign> Transformer::visit(
    std::unique_ptr<ContinuousAssign> node) {
  node->target = this->visit(std::move(node->target));
  node->value = this->visit(std::move(node->value));
  return node;
}

std::unique_ptr<Declaration> Transformer::visit(
    std::unique_ptr<Declaration> node) {
  if (auto ptr = dynamic_cast<Wire*>(node.get())) {
    node.release();
    return this->visit(std::unique_ptr<Wire>(ptr));
  }
  if (auto ptr = dynamic_cast<Reg*>(node.get())) {
    node.release();
    return this->visit(std::unique_ptr<Reg>(ptr));
  }
  throw std::runtime_error("Unreachable");  // LCOV_EXCL_LINE
  return node;                              // LCOV_EXCL_LINE
}

std::unique_ptr<BehavioralStatement> Transformer::visit(
    std::unique_ptr<BehavioralStatement> node) {
  if (auto ptr = dynamic_cast<BlockingAssign*>(node.get())) {
    node.release();
    return this->visit(std::unique_ptr<BlockingAssign>(ptr));
  }
  if (auto ptr = dynamic_cast<NonBlockingAssign*>(node.get())) {
    node.release();
    return this->visit(std::unique_ptr<NonBlockingAssign>(ptr));
  }
  if (auto ptr = dynamic_cast<CallStmt*>(node.get())) {
    node.release();
    return this->visit(std::unique_ptr<CallStmt>(ptr));
  }
  if (auto ptr = dynamic_cast<SingleLineComment*>(node.get())) {
    node.release();
    return this->visit(std::unique_ptr<SingleLineComment>(ptr));
  }
  if (auto ptr = dynamic_cast<BlockComment*>(node.get())) {
    node.release();
    return this->visit(std::unique_ptr<BlockComment>(ptr));
  }
  throw std::runtime_error("Unreachable");  // LCOV_EXCL_LINE
  return node;                              // LCOV_EXCL_LINE
}

std::unique_ptr<BlockingAssign> Transformer::visit(
    std::unique_ptr<BlockingAssign> node) {
  node->target = this->visit(std::move(node->target));
  node->value = this->visit(std::move(node->value));
  return node;
}

std::unique_ptr<NonBlockingAssign> Transformer::visit(
    std::unique_ptr<NonBlockingAssign> node) {
  node->target = this->visit(std::move(node->target));
  node->value = this->visit(std::move(node->value));
  return node;
}

std::unique_ptr<CallStmt> Transformer::visit(std::unique_ptr<CallStmt> node) {
  std::vector<std::unique_ptr<Expression>> new_args;
  for (auto&& expr : node->args) {
    new_args.push_back(this->visit(std::move(expr)));
  }
  node->args = std::move(new_args);
  return node;
}

std::unique_ptr<Star> Transformer::visit(std::unique_ptr<Star> node) {
  return node;
}

std::unique_ptr<Always> Transformer::visit(std::unique_ptr<Always> node) {
  std::vector<
      std::variant<std::unique_ptr<Identifier>, std::unique_ptr<PosEdge>,
                   std::unique_ptr<NegEdge>, std::unique_ptr<Star>>>
      new_sensitivity_list;
  for (auto&& item : node->sensitivity_list) {
    new_sensitivity_list.push_back(this->visit(std::move(item)));
  }
  node->sensitivity_list = std::move(new_sensitivity_list);
  std::vector<std::variant<std::unique_ptr<BehavioralStatement>,
                           std::unique_ptr<Declaration>>>
      new_body;
  for (auto&& item : node->body) {
    new_body.push_back(this->visit(std::move(item)));
  }
  node->body = std::move(new_body);
  return node;
}

std::unique_ptr<AbstractPort> Transformer::visit(
    std::unique_ptr<AbstractPort> node) {
  if (auto ptr = dynamic_cast<Port*>(node.get())) {
    node.release();
    return this->visit(std::unique_ptr<Port>(ptr));
  }
  if (auto ptr = dynamic_cast<StringPort*>(node.get())) {
    node.release();
    return this->visit(std::unique_ptr<StringPort>(ptr));
  }
  throw std::runtime_error("Unreachable");  // LCOV_EXCL_LINE
  return node;                              // LCOV_EXCL_LINE
}

std::unique_ptr<StructuralStatement> Transformer::visit(
    std::unique_ptr<StructuralStatement> node) {
  if (auto ptr = dynamic_cast<ModuleInstantiation*>(node.get())) {
    node.release();
    return this->visit(std::unique_ptr<ModuleInstantiation>(ptr));
  }
  if (auto ptr = dynamic_cast<ContinuousAssign*>(node.get())) {
    node.release();
    return this->visit(std::unique_ptr<ContinuousAssign>(ptr));
  }
  if (auto ptr = dynamic_cast<Always*>(node.get())) {
    node.release();
    return this->visit(std::unique_ptr<Always>(ptr));
  }
  if (auto ptr = dynamic_cast<SingleLineComment*>(node.get())) {
    node.release();
    return this->visit(std::unique_ptr<SingleLineComment>(ptr));
  }
  if (auto ptr = dynamic_cast<BlockComment*>(node.get())) {
    node.release();
    return this->visit(std::unique_ptr<BlockComment>(ptr));
  }
  if (auto ptr = dynamic_cast<InlineVerilog*>(node.get())) {
    node.release();
    return this->visit(std::unique_ptr<InlineVerilog>(ptr));
  }
  throw std::runtime_error("Unreachable");  // LCOV_EXCL_LINE
  return node;                              // LCOV_EXCL_LINE
}

std::unique_ptr<Module> Transformer::visit(std::unique_ptr<Module> node) {
  std::vector<std::unique_ptr<AbstractPort>> new_ports;
  for (auto&& item : node->ports) {
    new_ports.push_back(this->visit(std::move(item)));
  }
  node->ports = std::move(new_ports);
  for (auto&& param : node->parameters) {
    param.first = this->visit(std::move(param.first));
    param.second = this->visit(std::move(param.second));
  }
  std::vector<std::variant<std::unique_ptr<StructuralStatement>,
                           std::unique_ptr<Declaration>>>
      new_body;
  for (auto&& item : node->body) {
    new_body.push_back(this->visit(std::move(item)));
  }
  node->body = std::move(new_body);
  return node;
}

std::unique_ptr<StringBodyModule> Transformer::visit(
    std::unique_ptr<StringBodyModule> node) {
  std::vector<std::unique_ptr<AbstractPort>> new_ports;
  for (auto&& item : node->ports) {
    new_ports.push_back(this->visit(std::move(item)));
  }
  node->ports = std::move(new_ports);
  for (auto&& param : node->parameters) {
    param.first = this->visit(std::move(param.first));
    param.second = this->visit(std::move(param.second));
  }
  return node;
}

std::unique_ptr<StringModule> Transformer::visit(
    std::unique_ptr<StringModule> node) {
  return node;
}

std::unique_ptr<AbstractModule> Transformer::visit(
    std::unique_ptr<AbstractModule> node) {
  if (auto ptr = dynamic_cast<StringBodyModule*>(node.get())) {
    node.release();
    return this->visit(std::unique_ptr<StringBodyModule>(ptr));
  }
  if (auto ptr = dynamic_cast<Module*>(node.get())) {
    node.release();
    return this->visit(std::unique_ptr<Module>(ptr));
  }
  if (auto ptr = dynamic_cast<StringModule*>(node.get())) {
    node.release();
    return this->visit(std::unique_ptr<StringModule>(ptr));
  }
  throw std::runtime_error("Unreachable");  // LCOV_EXCL_LINE
  return node;                              // LCOV_EXCL_LINE
}

std::unique_ptr<File> Transformer::visit(std::unique_ptr<File> node) {
  std::vector<std::unique_ptr<AbstractModule>> new_modules;
  for (auto&& item : node->modules) {
    new_modules.push_back(this->visit(std::move(item)));
  }
  node->modules = std::move(new_modules);
  return node;
}

}  // namespace verilogAST
