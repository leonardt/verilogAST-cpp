#include "verilogAST/assign_inliner.hpp"
#include <iostream>

namespace verilogAST {

void Blacklister::blacklist_invalid_driver(std::unique_ptr<Identifier> node) {
  if (!assign_map.count(node->toString())) {
    // Not in assign map, means it's a module input, don't need to do anything
    // because it won't be inlined
    return;
  }
  auto driver = assign_map[node->toString()]->clone();
  // Can only inline if driven by identifier, index, or slice
  bool valid_driver = dynamic_cast<Identifier*>(driver.get()) ||
                      dynamic_cast<Index*>(driver.get()) ||
                      dynamic_cast<Slice*>(driver.get()) ||
                      dynamic_cast<NumericLiteral*>(driver.get());
  if (!valid_driver) {
    this->wire_blacklist.insert(node->value);
  } else if (auto ptr = dynamic_cast<Identifier*>(driver.get())) {
    // if driven by an id, we need to recursively blacklist any invalid
    // drivers, else they'll eventually get inlined into here
    driver.release();
    blacklist_invalid_driver(std::unique_ptr<Identifier>(ptr));
  }
}

std::unique_ptr<Identifier> Blacklister::visit(
    std::unique_ptr<Identifier> node) {
  if (this->blacklist) {
    blacklist_invalid_driver(node->clone());
  }
  return node;
}

std::unique_ptr<Slice> SliceBlacklister::visit(std::unique_ptr<Slice> node) {
  bool prev = this->blacklist;
  this->blacklist = true;
  node = Transformer::visit(std::move(node));
  // Restore prev value, since we could be nested inside an slice
  this->blacklist = prev;
  return node;
}

std::unique_ptr<Index> IndexBlacklister::visit(std::unique_ptr<Index> node) {
  bool prev = this->blacklist;
  this->blacklist = true;
  node = Transformer::visit(std::move(node));
  // Restore prev value, since we could be nested inside an index
  this->blacklist = prev;
  return node;
}

std::unique_ptr<ModuleInstantiation> ModuleInstanceBlacklister::visit(
    std::unique_ptr<ModuleInstantiation> node) {
  this->blacklist = true;
  for (auto&& conn : *node->connections) {
    conn.second = this->visit(std::move(conn.second));
  }
  this->blacklist = false;
  return node;
}

std::unique_ptr<Identifier> WireReadCounter::visit(
    std::unique_ptr<Identifier> node) {
  this->read_count[node->toString()]++;
  return node;
}

std::unique_ptr<Declaration> WireReadCounter::visit(
    std::unique_ptr<Declaration> node) {
  return node;
}

template <typename T>
std::unique_ptr<T> WireReadCounter::process_assign(std::unique_ptr<T> node) {
  node->value = this->visit(std::move(node->value));
  return node;
}

std::unique_ptr<ContinuousAssign> WireReadCounter::visit(
    std::unique_ptr<ContinuousAssign> node) {
  return this->process_assign(std::move(node));
}

std::unique_ptr<BlockingAssign> WireReadCounter::visit(
    std::unique_ptr<BlockingAssign> node) {
  return this->process_assign(std::move(node));
}

std::unique_ptr<Port> AssignMapBuilder::visit(std::unique_ptr<Port> node) {
  std::string port_str = std::visit(
      [](auto&& value) -> std::string {
        if (auto ptr = dynamic_cast<Identifier*>(value.get())) {
          return ptr->toString();
        } else if (auto ptr = dynamic_cast<Vector*>(value.get())) {
          return ptr->id->toString();
        }
        throw std::runtime_error("Unreachable");  // LCOV_EXCL_LINE
        return "";                                // LCOV_EXCL_LINE
      },
      node->value);
  if (node->direction != Direction::INPUT) {
    this->non_input_ports.insert(port_str);
    if (node->direction == Direction::OUTPUT) {
      this->output_ports.insert(port_str);
    }
  } else {
    this->input_ports.insert(port_str);
  }
  return node;
}
template <typename T>
std::unique_ptr<T> AssignMapBuilder::process_assign(std::unique_ptr<T> node) {
  node = Transformer::visit(std::move(node));
  std::string key =
      std::visit([](auto&& value) -> std::string { return value->toString(); },
                 node->target);
  this->assign_map[key] = node->value->clone();
  this->assign_count[key]++;
  return node;
}

std::unique_ptr<BlockingAssign> AssignMapBuilder::visit(
    std::unique_ptr<BlockingAssign> node) {
  return this->process_assign(std::move(node));
}

std::unique_ptr<ContinuousAssign> AssignMapBuilder::visit(
    std::unique_ptr<ContinuousAssign> node) {
  return this->process_assign(std::move(node));
}

bool AssignInliner::can_inline(std::string key) {
  if (this->wire_blacklist.count(key)) {
    return false;
  }
  auto it = assign_map.find(key);
  return it != assign_map.end() && (this->assign_count[key] == 1) &&
         (this->read_count[key] == 1 ||
          dynamic_cast<Identifier*>(it->second.get()) ||
          dynamic_cast<NumericLiteral*>(it->second.get()));
}

std::unique_ptr<Index> AssignInliner::visit(std::unique_ptr<Index> node) {
  if (std::holds_alternative<std::unique_ptr<Identifier>>(node->value)) {
    std::string key =
        std::get<std::unique_ptr<Identifier>>(node->value)->toString();
    if (this->can_inline(key)) {
      std::unique_ptr<Expression> value = this->visit(assign_map[key]->clone());
      if (auto ptr = dynamic_cast<Identifier*>(value.get())) {
        value.release();
        node->value = std::unique_ptr<Identifier>(ptr);
      }
    }
    return node;
  }
  return Transformer::visit(std::move(node));
}

std::unique_ptr<Expression> AssignInliner::visit(
    std::unique_ptr<Expression> node) {
  if (auto ptr = dynamic_cast<Identifier*>(node.get())) {
    node.release();
    std::unique_ptr<Identifier> id(ptr);
    std::string key = id->toString();
    if (this->can_inline(key)) {
      return this->visit(assign_map[key]->clone());
    }
    return id;
  }
  return Transformer::visit(std::move(node));
}

std::unique_ptr<Wire> AssignInliner::visit(std::unique_ptr<Wire> node) {
  bool remove = false;
  std::visit(
      [&](auto&& value) {
        if (auto ptr = dynamic_cast<Identifier*>(value.get())) {
          remove = this->can_inline(ptr->toString());
        } else if (auto ptr = dynamic_cast<Vector*>(value.get())) {
          remove = this->can_inline(ptr->id->toString());
        }
      },
      node->value);
  if (remove) {
    return std::unique_ptr<Wire>{};
  }
  node->value = this->visit(std::move(node->value));
  return node;
}

template <typename T>
std::unique_ptr<T> AssignInliner::process_assign(std::unique_ptr<T> node) {
  node->value = this->visit(std::move(node->value));
  std::string key =
      std::visit([](auto&& value) -> std::string { return value->toString(); },
                 node->target);
  bool remove = false;
  std::visit(
      [&](auto&& value) {
        if (auto ptr = dynamic_cast<Identifier*>(value.get())) {
          if (this->can_inline(key) && this->non_input_ports.count(key) == 0) {
            remove = true;
          } else if (this->inlined_outputs.count(ptr->toString())) {
            remove = true;
          };
        }
      },
      node->target);
  if (remove) {
    return std::unique_ptr<T>{};
  }
  return node;
}

std::unique_ptr<ContinuousAssign> AssignInliner::visit(
    std::unique_ptr<ContinuousAssign> node) {
  return this->process_assign(std::move(node));
}

std::unique_ptr<BlockingAssign> AssignInliner::visit(
    std::unique_ptr<BlockingAssign> node) {
  return this->process_assign(std::move(node));
}

std::vector<std::variant<std::unique_ptr<StructuralStatement>,
                         std::unique_ptr<Declaration>>>
AssignInliner::do_inline(
    std::vector<std::variant<std::unique_ptr<StructuralStatement>,
                             std::unique_ptr<Declaration>>>
        body) {
  std::vector<std::variant<std::unique_ptr<StructuralStatement>,
                           std::unique_ptr<Declaration>>>
      new_body;
  for (auto&& item : body) {
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
  return new_body;
}

std::unique_ptr<Module> AssignInliner::visit(std::unique_ptr<Module> node) {
  AssignMapBuilder builder(this->assign_count, this->assign_map,
                           this->non_input_ports, this->output_ports,
                           this->input_ports);
  node = builder.visit(std::move(node));

  WireReadCounter counter(this->read_count);
  node = counter.visit(std::move(node));

  IndexBlacklister index_blacklist(this->wire_blacklist, this->assign_map);
  node = index_blacklist.visit(std::move(node));

  SliceBlacklister slice_blacklist(this->wire_blacklist, this->assign_map);
  node = slice_blacklist.visit(std::move(node));

  ModuleInstanceBlacklister module_instance_blacklister(this->wire_blacklist,
                                                        this->assign_map);
  node = module_instance_blacklister.visit(std::move(node));

  std::vector<std::unique_ptr<AbstractPort>> new_ports;
  for (auto&& item : node->ports) {
    new_ports.push_back(this->visit(std::move(item)));
  }
  node->ports = std::move(new_ports);

  node->body = this->do_inline(std::move(node->body));
  // Now "reverse inline" output wires
  for (auto output : this->output_ports) {
    if (!this->assign_map.count(output)) {
      continue;
    }
    std::unique_ptr<Expression> value = this->assign_map[output]->clone();
    this->assign_map.erase(output);
    if (dynamic_cast<Identifier*>(value.get()) &&
        this->assign_count[value->toString()] == 0 &&
        this->input_ports.count(value->toString()) == 0 &&
        this->wire_blacklist.count(value->toString()) == 0) {
      this->assign_map[value->toString()] = make_id(output);
      this->assign_count[value->toString()]++;
      this->inlined_outputs.insert(output);
    }
  }
  node->body = this->do_inline(std::move(node->body));
  return node;
}

}  // namespace verilogAST
