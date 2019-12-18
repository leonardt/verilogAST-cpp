#include "verilogAST/assign_inliner.hpp"
#include <iostream>

namespace verilogAST {

std::unique_ptr<Identifier> WireReadCounter::visit(
    std::unique_ptr<Identifier> node) {
  this->read_count[node->toString()]++;
  return node;
}

std::unique_ptr<Declaration> WireReadCounter::visit(
    std::unique_ptr<Declaration> node) {
  return node;
}

std::unique_ptr<ContinuousAssign> WireReadCounter::visit(
    std::unique_ptr<ContinuousAssign> node) {
  node->value = this->visit(std::move(node->value));
  return node;
}

std::unique_ptr<ContinuousAssign> AssignMapBuilder::visit(
    std::unique_ptr<ContinuousAssign> node) {
  node = Transformer::visit(std::move(node));
  std::string key =
      std::visit([](auto&& value) -> std::string { return value->toString(); },
                 node->target);
  this->assign_map[key] = node->value->clone();
  this->assign_count[key]++;
  return node;
}

std::unique_ptr<Expression> AssignInliner::visit(
    std::unique_ptr<Expression> node) {
  if (auto ptr = dynamic_cast<Identifier*>(node.get())) {
    node.release();
    std::unique_ptr<Identifier> id(ptr);
    std::string key = id->toString();
    std::map<std::string, std::unique_ptr<Expression>>::iterator it =
        assign_map.find(key);
    if (it != assign_map.end() && (this->assign_count[key] == 1) &&
        (this->read_count[id->toString()] == 1 ||
         dynamic_cast<Identifier*>(it->second.get()))) {
      return it->second->clone();
    }
    return id;
  }
  return node;
}

std::unique_ptr<Wire> AssignInliner::visit(std::unique_ptr<Wire> node) {
  bool remove = false;
  std::visit(
      [&](auto&& value) {
        if (auto ptr = dynamic_cast<Identifier*>(value.get())) {
          std::string key = ptr->toString();
          std::map<std::string, std::unique_ptr<Expression>>::iterator it =
              this->assign_map.find(key);
          if (it != assign_map.end() && (this->assign_count[key] == 1) &&
              (this->read_count[key] == 1 ||
               dynamic_cast<Identifier*>(it->second.get())) &&
              this->non_input_ports.count(key) == 0) {
            remove = true;
          };
        } else if (auto ptr = dynamic_cast<Vector*>(value.get())) {
          std::string key = ptr->id->toString();
          std::map<std::string, std::unique_ptr<Expression>>::iterator it =
              this->assign_map.find(key);
          if (it != assign_map.end() && (this->assign_count[key] == 1) &&
              (this->read_count[key] == 1 ||
               dynamic_cast<Identifier*>(it->second.get())) &&
              this->non_input_ports.count(key) == 0) {
            remove = true;
          };
        }
      },
      node->value);
  if (remove) {
    return std::unique_ptr<Wire>{};
  }
  node->value = this->visit(std::move(node->value));
  return node;
}

std::unique_ptr<ContinuousAssign> AssignInliner::visit(
    std::unique_ptr<ContinuousAssign> node) {
  node->value = this->visit(std::move(node->value));
  std::string key =
      std::visit([](auto&& value) -> std::string { return value->toString(); },
                 node->target);
  bool remove = false;
  std::visit(
      [&](auto&& value) {
        if (auto ptr = dynamic_cast<Identifier*>(value.get())) {
          std::map<std::string, std::unique_ptr<Expression>>::iterator it =
              this->assign_map.find(ptr->toString());
          if (it != assign_map.end() && (this->assign_count[key] == 1) &&
              (this->read_count[ptr->toString()] == 1 ||
               dynamic_cast<Identifier*>(it->second.get())) &&
              this->non_input_ports.count(ptr->toString()) == 0) {
            remove = true;
            this->assign_map[key] = node->value->clone();
          };
        }
      },
      node->target);
  if (remove) {
    return std::unique_ptr<ContinuousAssign>{};
  }
  return node;
}

std::unique_ptr<Port> AssignInliner::visit(std::unique_ptr<Port> node) {
  if (node->direction != Direction::INPUT) {
    this->non_input_ports.insert(std::visit(
        [](auto&& value) -> std::string {
          if (auto ptr = dynamic_cast<Identifier*>(value.get())) {
            return ptr->toString();
          } else if (auto ptr = dynamic_cast<Vector*>(value.get())) {
            return ptr->id->toString();
          }
          throw std::runtime_error("Unreachable");  // LCOV_EXCL_LINE
          return "";                                // LCOV_EXCL_LINE
        },
        node->value));
  }
  return node;
}  // namespace verilogAST

std::unique_ptr<Module> AssignInliner::visit(std::unique_ptr<Module> node) {
  // std::map<std::string, std::unique_ptr<Expression>>
  //     assign_map;
  AssignMapBuilder builder(this->assign_count, this->assign_map);
  node = builder.visit(std::move(node));

  WireReadCounter counter(this->read_count);
  node = counter.visit(std::move(node));

  std::vector<std::unique_ptr<AbstractPort>> new_ports;
  for (auto&& item : node->ports) {
    new_ports.push_back(this->visit(std::move(item)));
  }
  node->ports = std::move(new_ports);

  std::vector<std::variant<std::unique_ptr<StructuralStatement>,
                           std::unique_ptr<Declaration>>>
      new_body;
  for (auto&& item : node->body) {
    std::variant<std::unique_ptr<StructuralStatement>,
                 std::unique_ptr<Declaration>>
        result = this->visit(std::move(item));
    bool is_null;
    std::visit(
        [&](auto&& value) {
          if (value) {
            is_null = false;
          } else {
            is_null = true;
          }
        },
        result);
    if (!is_null) {
      new_body.push_back(std::move(result));
    }
  }
  node->body = std::move(new_body);
  return node;
}

}  // namespace verilogAST
