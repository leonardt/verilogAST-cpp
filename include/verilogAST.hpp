#pragma once
#ifndef VERILOGAST_H
#define VERILOGAST_H

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

 public:
  NumericLiteral(std::string value, unsigned int size, bool _signed,
                 Radix radix)
      : value(value), size(size), _signed(_signed), radix(radix){};

  NumericLiteral(std::string value, unsigned int size, bool _signed)
      : value(value), size(size), _signed(_signed), radix(Radix::DECIMAL){};

  NumericLiteral(std::string value, unsigned int size)
      : value(value), size(size), _signed(false), radix(Radix::DECIMAL){};

  NumericLiteral(std::string value)
      : value(value), size(32), _signed(false), radix(Radix::DECIMAL){};
  std::string toString() override;
};

// TODO also need a string literal, as strings can be used as parameter values

class Identifier : public Expression {
  std::string value;

 public:
  Identifier(std::string value) : value(value){};

  std::string toString() override;
  ~Identifier(){};
};

class String : public Expression {
  std::string value;

 public:
  String(std::string value) : value(value){};

  std::string toString() override;
  ~String(){};
};

class Index : public Expression {
  std::unique_ptr<Identifier> id;
  std::unique_ptr<Expression> index;

 public:
  Index(std::unique_ptr<Identifier> id, std::unique_ptr<Expression> index)
      : id(std::move(id)), index(std::move(index)){};
  std::string toString() override;
  ~Index(){};
};

class Slice : public Expression {
  std::unique_ptr<Identifier> id;
  std::unique_ptr<Expression> high_index;
  std::unique_ptr<Expression> low_index;

 public:
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
  // TODO Bitwise Logical ops like &, |,
  LSHIFT,
  RSHIFT,
  AND,
  OR,
  EQ,
  NEQ,
  ADD,
  SUB,
  MUL,
  DIV,
  POW,
  MOD,
  ALSHIFT,
  ARSHIFT
};
}

class BinaryOp : public Expression {
  std::unique_ptr<Expression> left;
  BinOp::BinOp op;
  std::unique_ptr<Expression> right;

 public:
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
  std::unique_ptr<Expression> operand;

  UnOp::UnOp op;

 public:
  UnaryOp(std::unique_ptr<Expression> operand, UnOp::UnOp op)
      : operand(std::move(operand)), op(op){};
  std::string toString();
  ~UnaryOp(){};
};

class TernaryOp : public Expression {
  std::unique_ptr<Expression> cond;
  std::unique_ptr<Expression> true_value;
  std::unique_ptr<Expression> false_value;

 public:
  TernaryOp(std::unique_ptr<Expression> cond,
            std::unique_ptr<Expression> true_value,
            std::unique_ptr<Expression> false_value)
      : cond(std::move(cond)),
        true_value(std::move(true_value)),
        false_value(std::move(false_value)){};
  std::string toString();
  ~TernaryOp(){};
};

class NegEdge : public Expression {
  std::unique_ptr<Expression> value;

 public:
  NegEdge(std::unique_ptr<Expression> value) : value(std::move(value)){};
  std::string toString();
  ~NegEdge(){};
};

class PosEdge : public Expression {
  std::unique_ptr<Expression> value;

 public:
  PosEdge(std::unique_ptr<Expression> value) : value(std::move(value)){};
  std::string toString();
  ~PosEdge(){};
};

enum Direction { INPUT, OUTPUT, INOUT };

// TODO: Unify with declarations?
enum PortType { WIRE, REG };

class AbstractPort : public Node {};

class Vector : public Node {
  std::unique_ptr<Identifier> id;
  std::unique_ptr<Expression> msb;
  std::unique_ptr<Expression> lsb;

 public:
  Vector(std::unique_ptr<Identifier> id, std::unique_ptr<Expression> msb,
         std::unique_ptr<Expression> lsb)
      : id(std::move(id)), msb(std::move(msb)), lsb(std::move(lsb)){};
  std::string toString() override;
  ~Vector(){};
};

class Port : public AbstractPort {
  // Required
  // `<name>` or `<name>[n]` or `name[n:m]`
  std::variant<std::unique_ptr<Identifier>, std::unique_ptr<Vector>> value;

  // technically the following are optional (e.g. port direction/data type
  // can be declared in the body of the definition), but for now let's force
  // users to declare ports in a single, unified way for
  // simplicity/maintenance
  Direction direction;
  PortType data_type;

 public:
  Port(std::variant<std::unique_ptr<Identifier>, std::unique_ptr<Vector>> value,
       Direction direction, PortType data_type)
      : value(std::move(value)),
        direction(std::move(direction)),
        data_type(std::move(data_type)){};
  std::string toString();
  ~Port(){};
};

class StringPort : public AbstractPort {
  std::string value;

 public:
  StringPort(std::string value) : value(value){};
  std::string toString() { return value; };
  ~StringPort(){};
};

class Statement : public Node {};

class SingleLineComment : public Statement {
  std::string value;

 public:
  SingleLineComment(std::string value) : value(value){};
  std::string toString() { return "// " + value; };
  ~SingleLineComment(){};
};

class BlockComment : public Statement {
  std::string value;

 public:
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
  std::string module_name;

  // parameter,value
  Parameters parameters;

  std::string instance_name;

  // map from instance port names to connection expression
  // NOTE: anonymous style of module connections is not supported
  std::map<std::string,
           std::variant<std::unique_ptr<Identifier>, std::unique_ptr<Index>,
                        std::unique_ptr<Slice>>>
      connections;

 public:
  // TODO Need to make sure that the instance parameters are a subset of the
  // module parameters
  ModuleInstantiation(
      std::string module_name, Parameters parameters, std::string instance_name,
      std::map<std::string,
               std::variant<std::unique_ptr<Identifier>, std::unique_ptr<Index>,
                            std::unique_ptr<Slice>>>
          connections)
      : module_name(module_name),
        parameters(std::move(parameters)),
        instance_name(instance_name),
        connections(std::move(connections)){};
  std::string toString();
  ~ModuleInstantiation(){};
};

class Declaration : public Node {
 protected:
  std::string decl;
  std::variant<std::unique_ptr<Identifier>, std::unique_ptr<Index>,
               std::unique_ptr<Slice>, std::unique_ptr<Vector>>
      value;

  Declaration(std::variant<std::unique_ptr<Identifier>, std::unique_ptr<Index>,
                           std::unique_ptr<Slice>, std::unique_ptr<Vector>>
                  value,
              std::string decl)
      : decl(decl), value(std::move(value)){};

 public:
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
 protected:
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

 public:
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

class Star : Node {
 public:
  std::string toString() { return "*"; };
  ~Star(){};
};

class Always : public StructuralStatement {
  std::vector<
      std::variant<std::unique_ptr<Identifier>, std::unique_ptr<PosEdge>,
                   std::unique_ptr<NegEdge>, std::unique_ptr<Star>>>
      sensitivity_list;
  std::vector<std::variant<std::unique_ptr<BehavioralStatement>,
                           std::unique_ptr<Declaration>>>
      body;

 public:
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
 protected:
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

 public:
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
  std::string body;

 public:
  StringBodyModule(std::string name,
                   std::vector<std::unique_ptr<AbstractPort>> ports,
                   std::string body, Parameters parameters)
      : Module(name, std::move(ports), std::move(parameters)), body(body){};
  std::string toString();
  ~StringBodyModule(){};
};

class StringModule : public AbstractModule {
  std::string definition;

 public:
  StringModule(std::string definition) : definition(definition){};
  std::string toString() { return definition; };
  ~StringModule(){};
};

class File : public Node {
  std::vector<std::unique_ptr<AbstractModule>> modules;

 public:
  File(std::vector<std::unique_ptr<AbstractModule>> &modules)
      : modules(std::move(modules)){};
  std::string toString();
  ~File(){};
};

// Helper functions for constructing unique pointers
std::unique_ptr<Identifier> make_id(std::string name) {
  return std::make_unique<Identifier>(name);
}

std::unique_ptr<NumericLiteral> make_num(std::string val) {
  return std::make_unique<NumericLiteral>(val);
}

std::unique_ptr<BinaryOp> make_binop(std::unique_ptr<Expression> left,
                                     BinOp::BinOp op,
                                     std::unique_ptr<Expression> right) {
  return std::make_unique<BinaryOp>(std::move(left), op, std::move(right));
}

std::unique_ptr<Port> make_port(
    std::variant<std::unique_ptr<Identifier>, std::unique_ptr<Vector>> value,
    Direction direction, PortType data_type) {
  return std::make_unique<Port>(std::move(value), direction, data_type);
}

std::unique_ptr<Vector> make_vector(std::unique_ptr<Identifier> id,
                                    std::unique_ptr<Expression> msb,
                                    std::unique_ptr<Expression> lsb) {
  return std::make_unique<Vector>(std::move(id), std::move(msb),
                                  std::move(lsb));
}

}  // namespace verilogAST
#endif
