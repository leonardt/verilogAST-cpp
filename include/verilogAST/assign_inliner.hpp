#pragma once
#ifndef VERILOGAST_ASSIGN_INLINER_H
#define VERILOGAST_ASSIGN_INLINER_H
#include <set>
#include "verilogAST.hpp"
#include "verilogAST/transformer.hpp"

namespace verilogAST {

class AssignMapBuilder : public Transformer {
  std::map<std::string, int> &assign_count;
  std::map<std::string, std::unique_ptr<Expression>> &assign_map;
  std::set<std::string> &non_input_ports;
  std::set<std::string> &output_ports;
  std::set<std::string> &input_ports;

  template <typename T>
  std::unique_ptr<T> process_assign(std::unique_ptr<T> node);

 public:
  AssignMapBuilder(
      std::map<std::string, int> &assign_count,
      std::map<std::string, std::unique_ptr<Expression>> &assign_map,
      std::set<std::string> &non_input_ports,
      std::set<std::string> &output_ports, std::set<std::string> &input_ports)
      : assign_count(assign_count),
        assign_map(assign_map),
        non_input_ports(non_input_ports),
        output_ports(output_ports),
        input_ports(input_ports){};

  using Transformer::visit;
  virtual std::unique_ptr<Port> visit(std::unique_ptr<Port> node);
  virtual std::unique_ptr<ContinuousAssign> visit(
      std::unique_ptr<ContinuousAssign> node);
  virtual std::unique_ptr<BlockingAssign> visit(
      std::unique_ptr<BlockingAssign> node);
};

class WireReadCounter : public Transformer {
  // Counts number of times a wire is read
  //
  std::map<std::string, int> &read_count;

  template <typename T>
  std::unique_ptr<T> process_assign(std::unique_ptr<T> node);

 public:
  WireReadCounter(std::map<std::string, int> &read_count)
      : read_count(read_count){};

  using Transformer::visit;
  // Increment read count
  virtual std::unique_ptr<Identifier> visit(std::unique_ptr<Identifier> node);
  // Skip target of assign (not read)
  virtual std::unique_ptr<ContinuousAssign> visit(
      std::unique_ptr<ContinuousAssign> node);
  virtual std::unique_ptr<BlockingAssign> visit(
      std::unique_ptr<BlockingAssign> node);
  // Skip declarations (not read)
  virtual std::unique_ptr<Declaration> visit(std::unique_ptr<Declaration> node);
};

class Blacklister : public Transformer {
  std::set<std::string> &wire_blacklist;
  std::map<std::string, std::unique_ptr<Expression>> &assign_map;

 protected:
  bool blacklist = false;
  void blacklist_invalid_driver(std::unique_ptr<Identifier> node);

 public:
  Blacklister(std::set<std::string> &wire_blacklist,
              std::map<std::string, std::unique_ptr<Expression>> &assign_map)
      : wire_blacklist(wire_blacklist), assign_map(assign_map){};
  using Transformer::visit;
  virtual std::unique_ptr<Identifier> visit(std::unique_ptr<Identifier> node);
};

class SliceBlacklister : public Blacklister {
  // Prevent inling wires into slice nodes, e.g.
  // wire [7:0] x;
  // assign x = y + z;
  // assign w = x[4:0];
  //
  // Verilog does not support (y + z)[4:0]
 public:
  SliceBlacklister(
      std::set<std::string> &wire_blacklist,
      std::map<std::string, std::unique_ptr<Expression>> &assign_map)
      : Blacklister(wire_blacklist, assign_map){};
  using Blacklister::visit;
  virtual std::unique_ptr<Slice> visit(std::unique_ptr<Slice> node);
};

class IndexBlacklister : public Blacklister {
  // Prevent inling wires into index nodes, e.g.
  // wire x;
  // assign x = y + z;
  // assign w = x[0];
  //
  // Verilog does not support (y + z)[0]
 public:
  IndexBlacklister(
      std::set<std::string> &wire_blacklist,
      std::map<std::string, std::unique_ptr<Expression>> &assign_map)
      : Blacklister(wire_blacklist, assign_map){};
  using Blacklister::visit;
  virtual std::unique_ptr<Index> visit(std::unique_ptr<Index> node);
};

class ModuleInstanceBlacklister : public Blacklister {
  // Prevent inling wires into module instance nodes, e.g.
  // wire z;
  // assign b = a;
  // assign z = i + a;  // <--- not inlined into .w below
  // inner_module inner_module_inst (
  //     .c(a),
  //     .i(i),
  //     .w(z),
  //     .o(o)
  // );
  //
  // We can make this configurable, but for now we keep it as the default since
  // some tools do not support general expressions inside module instance
  // statements
 public:
  ModuleInstanceBlacklister(
      std::set<std::string> &wire_blacklist,
      std::map<std::string, std::unique_ptr<Expression>> &assign_map)
      : Blacklister(wire_blacklist, assign_map){};
  using Blacklister::visit;
  virtual std::unique_ptr<ModuleInstantiation> visit(
      std::unique_ptr<ModuleInstantiation> node);
};

class AssignInliner : public Transformer {
  std::map<std::string, int> read_count;
  std::map<std::string, int> assign_count;
  std::map<std::string, std::unique_ptr<Expression>> assign_map;
  std::set<std::string> non_input_ports;
  std::set<std::string> output_ports;
  std::set<std::string> input_ports;
  std::set<std::string> inlined_outputs;
  std::set<std::string> wire_blacklist;

  std::vector<std::variant<std::unique_ptr<StructuralStatement>,
                           std::unique_ptr<Declaration>>>
  do_inline(std::vector<std::variant<std::unique_ptr<StructuralStatement>,
                                     std::unique_ptr<Declaration>>>
                body);

  bool can_inline(std::string key);

  template <typename T>
  std::unique_ptr<T> process_assign(std::unique_ptr<T> node);

 public:
  AssignInliner() : wire_blacklist(){};
  explicit AssignInliner(std::set<std::string> wire_blacklist)
      : wire_blacklist(wire_blacklist){};
  using Transformer::visit;
  virtual std::unique_ptr<Expression> visit(std::unique_ptr<Expression> node);
  virtual std::unique_ptr<Index> visit(std::unique_ptr<Index> node);
  virtual std::unique_ptr<ContinuousAssign> visit(
      std::unique_ptr<ContinuousAssign> node);
  virtual std::unique_ptr<BlockingAssign> visit(
      std::unique_ptr<BlockingAssign> node);
  virtual std::unique_ptr<Wire> visit(std::unique_ptr<Wire> node);
  virtual std::unique_ptr<Module> visit(std::unique_ptr<Module> node);
};

}  // namespace verilogAST
#endif
