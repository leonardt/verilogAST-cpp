#pragma once
#ifndef VERILOGAST_H
#define VERILOGAST_H

#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <utility>  // std::pair
#include <variant>
#include <vector>

namespace verilogAST {

class Node {
 public:
  virtual std::string toString() = 0;
  virtual ~Node() = default;
};

class Expression : public Node {
 public:
  virtual std::string toString() = 0;
  virtual ~Expression() = default;
};

enum Radix { BINARY, OCTAL, HEX, DECIMAL };

class NumericLiteral : public Expression {
 public:
  /// For now, we model values as strings because it depends on their radix
  // (alternatively, we could store an unsigned integer representation and
  //  convert it during code generation)
  //
  // TODO Maybe add special toString logic for the default case? E.g. if we're
  // generating a 32 bit unsigned decimal literal (commonly used for indexing
  // into ports) then we don't need to generate the "32'd" prefix
  std::string value;
  unsigned int size;  // default 32
  bool _signed;       // default false
  Radix radix;        // default decimal

  NumericLiteral(std::string value, unsigned int size, bool _signed,
                 Radix radix)
      : value(value), size(size), _signed(_signed), radix(radix){};

  NumericLiteral(std::string value, unsigned int size, bool _signed)
      : value(value), size(size), _signed(_signed), radix(Radix::DECIMAL){};

  NumericLiteral(std::string value, unsigned int size)
      : value(value), size(size), _signed(false), radix(Radix::DECIMAL){};

  NumericLiteral(std::string value)
      : value(value), size(32), _signed(false), radix(Radix::DECIMAL){};

  NumericLiteral(std::string value, Radix radix)
      : value(value), size(32), _signed(false), radix(radix){};
  std::string toString() override;
};

// TODO also need a string literal, as strings can be used as parameter values

class Identifier : public Expression {
 public:
  std::string value;

  Identifier(std::string value) : value(value){};

  std::string toString() override;
  ~Identifier(){};
};

class String : public Expression {
 public:
  std::string value;

  String(std::string value) : value(value){};

  std::string toString() override;
  ~String(){};
};

class Index : public Expression {
 public:
  std::unique_ptr<Identifier> id;
  std::unique_ptr<Expression> index;

  Index(std::unique_ptr<Identifier> id, std::unique_ptr<Expression> index)
      : id(std::move(id)), index(std::move(index)){};
  std::string toString() override;
  ~Index(){};
};

class Slice : public Expression {
 public:
  std::unique_ptr<Identifier> id;
  std::unique_ptr<Expression> high_index;
  std::unique_ptr<Expression> low_index;

  Slice(std::unique_ptr<Identifier> id, std::unique_ptr<Expression> high_index,
        std::unique_ptr<Expression> low_index)
      : id(std::move(id)),
        high_index(std::move(high_index)),
        low_index(std::move(low_index)){};
  std::string toString() override;
  ~Slice(){};
};

namespace BinOp {
enum BinOp {
  LSHIFT,
  RSHIFT,
  AND,
  LAND,
  OR,
  LOR,
  XOR,
  EQ,
  NEQ,
  ADD,
  SUB,
  MUL,
  DIV,
  POW,
  MOD,
  ALSHIFT,
  ARSHIFT,
  LT,
  LTE,
  GT,
  GTE
};
}

class BinaryOp : public Expression {
 public:
  std::unique_ptr<Expression> left;
  BinOp::BinOp op;
  std::unique_ptr<Expression> right;

  BinaryOp(std::unique_ptr<Expression> left, BinOp::BinOp op,
           std::unique_ptr<Expression> right)
      : left(std::move(left)), op(op), right(std::move(right)){};
  std::string toString() override;
  ~BinaryOp(){};
};

namespace UnOp {
enum UnOp {
  NOT,
  INVERT,
  AND,
  NAND,
  OR,
  NOR,
  XOR,
  NXOR,
  XNOR,  // TODO ~^ vs ^~?
  PLUS,
  MINUS
};
}

class UnaryOp : public Expression {
 public:
  std::unique_ptr<Expression> operand;

  UnOp::UnOp op;

  UnaryOp(std::unique_ptr<Expression> operand, UnOp::UnOp op)
      : operand(std::move(operand)), op(op){};
  std::string toString();
  ~UnaryOp(){};
};

class TernaryOp : public Expression {
 public:
  std::unique_ptr<Expression> cond;
  std::unique_ptr<Expression> true_value;
  std::unique_ptr<Expression> false_value;

  TernaryOp(std::unique_ptr<Expression> cond,
            std::unique_ptr<Expression> true_value,
            std::unique_ptr<Expression> false_value)
      : cond(std::move(cond)),
        true_value(std::move(true_value)),
        false_value(std::move(false_value)){};
  std::string toString();
  ~TernaryOp(){};
};

class Concat : public Expression {
 public:
  std::vector<std::unique_ptr<Expression>> args;

  Concat(std::vector<std::unique_ptr<Expression>> args)
      : args(std::move(args)){};
  std::string toString();
};

class Replicate : public Expression {
 public:
  std::unique_ptr<Expression> num;
  std::unique_ptr<Expression> value;

  Replicate(std::unique_ptr<Expression> num, std::unique_ptr<Expression> value)
      : num(std::move(num)), value(std::move(value)){};
  std::string toString();
};

class NegEdge : public Node {
 public:
  std::unique_ptr<Identifier> value;

  NegEdge(std::unique_ptr<Identifier> value) : value(std::move(value)){};
  std::string toString();
  ~NegEdge(){};
};

class PosEdge : public Node {
 public:
  std::unique_ptr<Identifier> value;

  PosEdge(std::unique_ptr<Identifier> value) : value(std::move(value)){};
  std::string toString();
  ~PosEdge(){};
};

class Call {
 public:
  std::string func;
  std::vector<std::unique_ptr<Expression>> args;

  Call(std::string func, std::vector<std::unique_ptr<Expression>> args)
      : func(func), args(std::move(args)){};
  std::string toString();
  ~Call(){};
};

class CallExpr : public Expression, public Call {
 public:
  CallExpr(std::string func, std::vector<std::unique_ptr<Expression>> args)
      : Call(std::move(func), std::move(args)){};
  std::string toString() { return Call::toString(); };
};

enum Direction { INPUT, OUTPUT, INOUT };

// TODO: Unify with declarations?
enum PortType { WIRE, REG };

class AbstractPort : public Node {};

class Vector : public Node {
 public:
  std::unique_ptr<Identifier> id;
  std::unique_ptr<Expression> msb;
  std::unique_ptr<Expression> lsb;

  Vector(std::unique_ptr<Identifier> id, std::unique_ptr<Expression> msb,
         std::unique_ptr<Expression> lsb)
      : id(std::move(id)), msb(std::move(msb)), lsb(std::move(lsb)){};
  std::string toString() override;
  ~Vector(){};
};

class Port : public AbstractPort {
 public:
  // Required
  // `<name>` or `<name>[n]` or `name[n:m]`
  std::variant<std::unique_ptr<Identifier>, std::unique_ptr<Vector>> value;

  // technically the following are optional (e.g. port direction/data type
  // can be declared in the body of the definition), but for now let's force
  // users to declare ports in a single, unified way for
  // simplicity/maintenance
  Direction direction;
  PortType data_type;

  Port(std::variant<std::unique_ptr<Identifier>, std::unique_ptr<Vector>> value,
       Direction direction, PortType data_type)
      : value(std::move(value)),
        direction(std::move(direction)),
        data_type(std::move(data_type)){};
  std::string toString();
  ~Port(){};
};

class StringPort : public AbstractPort {
 public:
  std::string value;

  StringPort(std::string value) : value(value){};
  std::string toString() { return value; };
  ~StringPort(){};
};

class Statement : public Node {};

class SingleLineComment : public Statement {
 public:
  std::string value;

  SingleLineComment(std::string value) : value(value){};
  std::string toString() { return "// " + value; };
  ~SingleLineComment(){};
};

class BlockComment : public Statement {
 public:
  std::string value;

  BlockComment(std::string value) : value(value){};
  std::string toString() { return "/*\n" + value + "\n*/"; };
  ~BlockComment(){};
};

class BehavioralStatement : public Statement {};
class StructuralStatement : public Statement {};

typedef std::vector<
    std::pair<std::unique_ptr<Identifier>, std::unique_ptr<Expression>>>
    Parameters;

class ModuleInstantiation : public StructuralStatement {
 public:
  std::string module_name;

  // parameter,value
  Parameters parameters;

  std::string instance_name;

  // map from instance port names to connection expression
  // NOTE: anonymous style of module connections is not supported
  std::map<std::string,
           std::variant<std::unique_ptr<Identifier>, std::unique_ptr<Index>,
                        std::unique_ptr<Slice>, std::unique_ptr<Concat>>>
      connections;

  // TODO Need to make sure that the instance parameters are a subset of the
  // module parameters
  ModuleInstantiation(
      std::string module_name, Parameters parameters, std::string instance_name,
      std::map<std::string,
               std::variant<std::unique_ptr<Identifier>, std::unique_ptr<Index>,
                            std::unique_ptr<Slice>, std::unique_ptr<Concat>>>
          connections)
      : module_name(module_name),
        parameters(std::move(parameters)),
        instance_name(instance_name),
        connections(std::move(connections)){};
  std::string toString();
  ~ModuleInstantiation(){};
};

class Declaration : public Node {
 public:
  std::string decl;
  std::variant<std::unique_ptr<Identifier>, std::unique_ptr<Index>,
               std::unique_ptr<Slice>, std::unique_ptr<Vector>>
      value;

  Declaration(std::variant<std::unique_ptr<Identifier>, std::unique_ptr<Index>,
                           std::unique_ptr<Slice>, std::unique_ptr<Vector>>
                  value,
              std::string decl)
      : decl(decl), value(std::move(value)){};

  std::string toString();
  virtual ~Declaration() = default;
};

class Wire : public Declaration {
 public:
  Wire(std::variant<std::unique_ptr<Identifier>, std::unique_ptr<Index>,
                    std::unique_ptr<Slice>, std::unique_ptr<Vector>>
           value)
      : Declaration(std::move(value), "wire"){};
  ~Wire(){};
};

class Reg : public Declaration {
 public:
  Reg(std::variant<std::unique_ptr<Identifier>, std::unique_ptr<Index>,
                   std::unique_ptr<Slice>, std::unique_ptr<Vector>>
          value)
      : Declaration(std::move(value), "reg"){};
  ~Reg(){};
};

class Assign : public Node {
 public:
  std::variant<std::unique_ptr<Identifier>, std::unique_ptr<Index>,
               std::unique_ptr<Slice>>
      target;
  std::unique_ptr<Expression> value;
  std::string prefix;
  std::string symbol;

  Assign(std::variant<std::unique_ptr<Identifier>, std::unique_ptr<Index>,
                      std::unique_ptr<Slice>>
             target,
         std::unique_ptr<Expression> value, std::string prefix)
      : target(std::move(target)),
        value(std::move(value)),
        prefix(prefix),
        symbol("="){};
  Assign(std::variant<std::unique_ptr<Identifier>, std::unique_ptr<Index>,
                      std::unique_ptr<Slice>>
             target,
         std::unique_ptr<Expression> value, std::string prefix,
         std::string symbol)
      : target(std::move(target)),
        value(std::move(value)),
        prefix(prefix),
        symbol(symbol){};

  std::string toString();
  virtual ~Assign() = default;
};

class ContinuousAssign : public StructuralStatement, public Assign {
 public:
  ContinuousAssign(std::variant<std::unique_ptr<Identifier>,
                                std::unique_ptr<Index>, std::unique_ptr<Slice>>
                       target,
                   std::unique_ptr<Expression> value)
      : Assign(std::move(target), std::move(value), "assign "){};
  // Multiple inheritance forces us to have to explicitly state this?
  std::string toString() { return Assign::toString(); };
  ~ContinuousAssign(){};
};

class BehavioralAssign : public BehavioralStatement {};

class BlockingAssign : public BehavioralAssign, public Assign {
 public:
  BlockingAssign(std::variant<std::unique_ptr<Identifier>,
                              std::unique_ptr<Index>, std::unique_ptr<Slice>>
                     target,
                 std::unique_ptr<Expression> value)
      : Assign(std::move(target), std::move(value), ""){};
  // Multiple inheritance forces us to have to explicitly state this?
  std::string toString() { return Assign::toString(); };
  ~BlockingAssign(){};
};

class NonBlockingAssign : public BehavioralAssign, public Assign {
 public:
  NonBlockingAssign(std::variant<std::unique_ptr<Identifier>,
                                 std::unique_ptr<Index>, std::unique_ptr<Slice>>
                        target,
                    std::unique_ptr<Expression> value)
      : Assign(std::move(target), std::move(value), "", "<="){};
  // Multiple inheritance forces us to have to explicitly state this?
  std::string toString() { return Assign::toString(); };
  ~NonBlockingAssign(){};
};

class CallStmt : public BehavioralStatement, public Call {
 public:
  CallStmt(std::string func, std::vector<std::unique_ptr<Expression>> args)
      : Call(std::move(func), std::move(args)){};
  std::string toString() { return Call::toString() + ";"; };
};

class Star : Node {
 public:
  std::string toString() { return "*"; };
  ~Star(){};
};

class Always : public StructuralStatement {
 public:
  std::vector<
      std::variant<std::unique_ptr<Identifier>, std::unique_ptr<PosEdge>,
                   std::unique_ptr<NegEdge>, std::unique_ptr<Star>>>
      sensitivity_list;
  std::vector<std::variant<std::unique_ptr<BehavioralStatement>,
                           std::unique_ptr<Declaration>>>
      body;

  Always(std::vector<
             std::variant<std::unique_ptr<Identifier>, std::unique_ptr<PosEdge>,
                          std::unique_ptr<NegEdge>, std::unique_ptr<Star>>>
             sensitivity_list,
         std::vector<std::variant<std::unique_ptr<BehavioralStatement>,
                                  std::unique_ptr<Declaration>>>
             body)
      : body(std::move(body)) {
    if (sensitivity_list.empty()) {
      throw std::runtime_error(
          "vAST::Always expects non-empty sensitivity list");
    }
    this->sensitivity_list = std::move(sensitivity_list);
  };
  std::string toString();
  ~Always(){};
};

class AbstractModule : public Node {};

class Module : public AbstractModule {
 public:
  std::string name;
  std::vector<std::unique_ptr<AbstractPort>> ports;
  std::vector<std::variant<std::unique_ptr<StructuralStatement>,
                           std::unique_ptr<Declaration>>>
      body;
  Parameters parameters;
  std::string emitModuleHeader();
  // Protected initializer that is used by the StringBodyModule subclass which
  // overrides the `body` field (but reuses the other fields)
  Module(std::string name, std::vector<std::unique_ptr<AbstractPort>> ports,
         Parameters parameters)
      : name(name),
        ports(std::move(ports)),
        parameters(std::move(parameters)){};

  Module(std::string name, std::vector<std::unique_ptr<AbstractPort>> ports,
         std::vector<std::variant<std::unique_ptr<StructuralStatement>,
                                  std::unique_ptr<Declaration>>>
             body,
         Parameters parameters)
      : name(name),
        ports(std::move(ports)),
        body(std::move(body)),
        parameters(std::move(parameters)){};

  std::string toString();
  ~Module(){};
};

class StringBodyModule : public Module {
 public:
  std::string body;

  StringBodyModule(std::string name,
                   std::vector<std::unique_ptr<AbstractPort>> ports,
                   std::string body, Parameters parameters)
      : Module(name, std::move(ports), std::move(parameters)), body(body){};
  std::string toString();
  ~StringBodyModule(){};
};

class StringModule : public AbstractModule {
 public:
  std::string definition;

  StringModule(std::string definition) : definition(definition){};
  std::string toString() { return definition; };
  ~StringModule(){};
};

class File : public Node {
 public:
  std::vector<std::unique_ptr<AbstractModule>> modules;

  File(std::vector<std::unique_ptr<AbstractModule>>& modules)
      : modules(std::move(modules)){};
  std::string toString();
  ~File(){};
};

// Helper functions for constructing unique pointers
std::unique_ptr<Identifier> make_id(std::string name);

std::unique_ptr<NumericLiteral> make_num(std::string val);

std::unique_ptr<BinaryOp> make_binop(std::unique_ptr<Expression> left,
                                     BinOp::BinOp op,
                                     std::unique_ptr<Expression> right);

std::unique_ptr<Port> make_port(
    std::variant<std::unique_ptr<Identifier>, std::unique_ptr<Vector>> value,
    Direction direction, PortType data_type);

std::unique_ptr<Vector> make_vector(std::unique_ptr<Identifier> id,
                                    std::unique_ptr<Expression> msb,
                                    std::unique_ptr<Expression> lsb);

class Transformer {
 public:
  virtual std::unique_ptr<Expression> visit(std::unique_ptr<Expression> node) {
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
    return node;  // LCOV_EXCL_LINE
  };

  virtual std::unique_ptr<NumericLiteral> visit(
      std::unique_ptr<NumericLiteral> node) {
    return node;
  };

  virtual std::unique_ptr<Identifier> visit(std::unique_ptr<Identifier> node) {
    return node;
  };

  virtual std::unique_ptr<String> visit(std::unique_ptr<String> node) {
    return node;
  };

  virtual std::unique_ptr<Index> visit(std::unique_ptr<Index> node) {
    node->id = this->visit(std::move(node->id));
    node->index = this->visit(std::move(node->index));
    return node;
  };

  virtual std::unique_ptr<Slice> visit(std::unique_ptr<Slice> node) {
    node->id = this->visit(std::move(node->id));
    node->high_index = this->visit(std::move(node->high_index));
    node->low_index = this->visit(std::move(node->low_index));
    return node;
  };

  virtual std::unique_ptr<BinaryOp> visit(std::unique_ptr<BinaryOp> node) {
    node->left = this->visit(std::move(node->left));
    node->right = this->visit(std::move(node->right));
    return node;
  };

  virtual std::unique_ptr<UnaryOp> visit(std::unique_ptr<UnaryOp> node) {
    node->operand = this->visit(std::move(node->operand));
    return node;
  };

  virtual std::unique_ptr<TernaryOp> visit(std::unique_ptr<TernaryOp> node) {
    node->cond = this->visit(std::move(node->cond));
    node->true_value = this->visit(std::move(node->true_value));
    node->false_value = this->visit(std::move(node->false_value));
    return node;
  };

  virtual std::unique_ptr<Concat> visit(std::unique_ptr<Concat> node) {
    std::vector<std::unique_ptr<Expression>> new_args;
    for (auto&& expr : node->args) {
      new_args.push_back(this->visit(std::move(expr)));
    }
    node->args = std::move(new_args);
    return node;
  };

  virtual std::unique_ptr<Replicate> visit(std::unique_ptr<Replicate> node) {
    node->num = this->visit(std::move(node->num));
    node->value = this->visit(std::move(node->value));
    return node;
  };

  virtual std::unique_ptr<NegEdge> visit(std::unique_ptr<NegEdge> node) {
    node->value = this->visit(std::move(node->value));
    return node;
  };

  virtual std::unique_ptr<PosEdge> visit(std::unique_ptr<PosEdge> node) {
    node->value = this->visit(std::move(node->value));
    return node;
  };

  virtual std::unique_ptr<CallExpr> visit(std::unique_ptr<CallExpr> node) {
    std::vector<std::unique_ptr<Expression>> new_args;
    for (auto&& expr : node->args) {
      new_args.push_back(this->visit(std::move(expr)));
    }
    node->args = std::move(new_args);
    return node;
  };

  virtual std::variant<std::unique_ptr<Identifier>, std::unique_ptr<Vector>>
  visit(
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
          return std::move(value);  // LCOV_EXCL_LINE
        },
        node);
  };

  virtual std::unique_ptr<Vector> visit(std::unique_ptr<Vector> node) {
    node->id = this->visit(std::move(node->id));
    node->msb = this->visit(std::move(node->msb));
    node->lsb = this->visit(std::move(node->lsb));
    return node;
  };

  virtual std::unique_ptr<Port> visit(std::unique_ptr<Port> node) {
    node->value = this->visit(std::move(node->value));
    return node;
  };

  virtual std::unique_ptr<StringPort> visit(std::unique_ptr<StringPort> node) {
    return node;
  };

  virtual std::unique_ptr<SingleLineComment> visit(
      std::unique_ptr<SingleLineComment> node) {
    return node;
  };

  virtual std::unique_ptr<BlockComment> visit(
      std::unique_ptr<BlockComment> node) {
    return node;
  };

  template <typename T>
  T visit(T node) {
    return std::visit(
        [&](auto&& value) -> T { return this->visit(std::move(value)); }, node);
  }

  virtual std::unique_ptr<ModuleInstantiation> visit(
      std::unique_ptr<ModuleInstantiation> node) {
    for (auto&& conn : node->connections) {
      conn.second = this->visit(std::move(conn.second));
    }
    for (auto&& param : node->parameters) {
      param.first = this->visit(std::move(param.first));
      param.second = this->visit(std::move(param.second));
    }
    return node;
  };

  virtual std::unique_ptr<Wire> visit(std::unique_ptr<Wire> node) {
    node->value = this->visit(std::move(node->value));
    return node;
  };

  virtual std::unique_ptr<Reg> visit(std::unique_ptr<Reg> node) {
    node->value = this->visit(std::move(node->value));
    return node;
  };

  virtual std::unique_ptr<ContinuousAssign> visit(
      std::unique_ptr<ContinuousAssign> node) {
    node->target = this->visit(std::move(node->target));
    node->value = this->visit(std::move(node->value));
    return node;
  };

  virtual std::unique_ptr<Declaration> visit(
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
    return node;  // LCOV_EXCL_LINE
  };

  virtual std::unique_ptr<BehavioralStatement> visit(
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
    throw std::runtime_error("Unreachable");  // LCOV_EXCL_LINE
    return node;  // LCOV_EXCL_LINE
  };

  virtual std::unique_ptr<BlockingAssign> visit(
      std::unique_ptr<BlockingAssign> node) {
    node->target = this->visit(std::move(node->target));
    node->value = this->visit(std::move(node->value));
    return node;
  };

  virtual std::unique_ptr<NonBlockingAssign> visit(
      std::unique_ptr<NonBlockingAssign> node) {
    node->target = this->visit(std::move(node->target));
    node->value = this->visit(std::move(node->value));
    return node;
  };

  virtual std::unique_ptr<CallStmt> visit(std::unique_ptr<CallStmt> node) {
    std::vector<std::unique_ptr<Expression>> new_args;
    for (auto&& expr : node->args) {
      new_args.push_back(this->visit(std::move(expr)));
    }
    node->args = std::move(new_args);
    return node;
  };

  virtual std::unique_ptr<Star> visit(std::unique_ptr<Star> node) {
    return node;
  };

  virtual std::unique_ptr<Always> visit(std::unique_ptr<Always> node) {
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
  };

  virtual std::unique_ptr<AbstractPort> visit(
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
    return node;  // LCOV_EXCL_LINE
  };

  virtual std::unique_ptr<StructuralStatement> visit(
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
    throw std::runtime_error("Unreachable");  // LCOV_EXCL_LINE
    return node;  // LCOV_EXCL_LINE
  };

  virtual std::unique_ptr<Module> visit(std::unique_ptr<Module> node) {
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
  };

  virtual std::unique_ptr<StringBodyModule> visit(
      std::unique_ptr<StringBodyModule> node) {
    return node;
  };

  virtual std::unique_ptr<StringModule> visit(
      std::unique_ptr<StringModule> node) {
    return node;
  };

  virtual std::unique_ptr<AbstractModule> visit(
      std::unique_ptr<AbstractModule> node) {
    if (auto ptr = dynamic_cast<Module*>(node.get())) {
      node.release();
      return this->visit(std::unique_ptr<Module>(ptr));
    }
    if (auto ptr = dynamic_cast<StringModule*>(node.get())) {
      node.release();
      return this->visit(std::unique_ptr<StringModule>(ptr));
    }
    if (auto ptr = dynamic_cast<StringBodyModule*>(node.get())) {
      node.release();
      return this->visit(std::unique_ptr<StringBodyModule>(ptr));
    }
    throw std::runtime_error("Unreachable");  // LCOV_EXCL_LINE
    return node;  // LCOV_EXCL_LINE
  };

  virtual std::unique_ptr<File> visit(std::unique_ptr<File> node) {
    std::vector<std::unique_ptr<AbstractModule>> new_modules;
    for (auto&& item : node->modules) {
      new_modules.push_back(this->visit(std::move(item)));
    }
    node->modules = std::move(new_modules);
    return node;
  };
};
}  // namespace verilogAST
#endif
