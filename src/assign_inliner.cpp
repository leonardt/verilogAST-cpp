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
  return node;
}

std::unique_ptr<Expression> AssignInliner::visit(
    std::unique_ptr<Expression> node) {
  if (auto ptr = dynamic_cast<Identifier*>(node.get())) {
    node.release();
    std::unique_ptr<Identifier> id(ptr);
    std::map<std::string, std::unique_ptr<Expression>>::iterator it =
        assign_map.find(id->toString());
    if (it != assign_map.end() &&
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
          std::map<std::string, std::unique_ptr<Expression>>::iterator it =
              this->assign_map.find(ptr->toString());
          if (it != this->assign_map.end() &&
              (this->read_count[ptr->toString()] == 1 ||
               dynamic_cast<Identifier*>(it->second.get())) &&
              this->non_input_ports.count(ptr->toString()) == 0) {
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
  this->assign_map[key] = node->value->clone();
  bool remove = false;
  std::visit(
      [&](auto&& value) {
        if (auto ptr = dynamic_cast<Identifier*>(value.get())) {
          std::map<std::string, std::unique_ptr<Expression>>::iterator it =
              this->assign_map.find(ptr->toString());
          if (it != this->assign_map.end() &&
              (this->read_count[ptr->toString()] == 1 ||
               dynamic_cast<Identifier*>(it->second.get())) &&
              this->non_input_ports.count(ptr->toString()) == 0) {
            remove = true;
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
  AssignMapBuilder builder(this->assign_map);
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
  // for (auto& stmt : node->body) {
  //   std::visit(
  //       [&](auto&& value) {
  //         if (auto ptr = dynamic_cast<ContinuousAssign*>(value.get())) {
  //           value.release();
  //           std::unique_ptr<ContinuousAssign> assign_stmt(ptr);
  //           if (auto ptr = dynamic_cast<Identifier*>(
  //                   assign_stmt->target.get())) {
  //             std::unique_ptr<Identifier> assign_stmt(ptr);
  //             if (collector.read_count()) {
  //               new_body.push_back(std::move(assign_stmt));
  //             }
  //           } else {
  //             new_body.push_back(std::move(assign_stmt));
  //           }
  //         } else if (auto ptr = dynamic_cast<Wire*>(value.get())) {
  //           value.release();
  //           std::unique_ptr<Wire> wire_stmt(ptr);
  //           WireReadCollector inner_collector;
  //           wire_stmt->value =
  //               inner_collector.visit(std::move(wire_stmt->value));
  //           ASSERT(inner_collector.reads.size() == 1,
  //                  "Should have exactly one read");
  //           if (collector.reads.count(*inner_collector.reads.begin())) {
  //             new_body.push_back(std::move(wire_stmt));
  //           }
  //         } else {
  //           new_body.push_back(std::move(value));
  //         }
  //       },
  //       std::move(stmt));
  // }
  node->body = std::move(new_body);
  return node;
}  // namespace verilogAST

}  // namespace verilogAST