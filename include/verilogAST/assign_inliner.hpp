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
};

class WireReadCounter : public Transformer {
  // Counts number of times a wire is read
  //
  std::map<std::string, int> &read_count;

 public:
  WireReadCounter(std::map<std::string, int> &read_count)
      : read_count(read_count){};

  using Transformer::visit;
  // Increment read count
  virtual std::unique_ptr<Identifier> visit(std::unique_ptr<Identifier> node);
  // Skip target of assign (not read)
  virtual std::unique_ptr<ContinuousAssign> visit(
      std::unique_ptr<ContinuousAssign> node);
  // Skip declarations (not read)
  virtual std::unique_ptr<Declaration> visit(std::unique_ptr<Declaration> node);
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

 public:
  AssignInliner() : wire_blacklist() {};
  AssignInliner(std::set<std::string> wire_blacklist) :
      wire_blacklist(wire_blacklist) {};
  using Transformer::visit;
  virtual std::unique_ptr<Expression> visit(std::unique_ptr<Expression> node);
  virtual std::unique_ptr<ContinuousAssign> visit(
      std::unique_ptr<ContinuousAssign> node);
  virtual std::unique_ptr<Wire> visit(std::unique_ptr<Wire> node);
  virtual std::unique_ptr<Module> visit(std::unique_ptr<Module> node);
};

}  // namespace verilogAST
#endif
