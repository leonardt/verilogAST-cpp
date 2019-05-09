#include <map>
#include <string>
#include <utility>  // std::pair
#include <variant>
#include <vector>

namespace verilogAST {

class Node {
 public:
  virtual std::string toString() = 0;
};

class Expression : public Node {
 public:
  virtual std::string toString() = 0;
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

class Identifier : public Expression {
  std::string value;

 public:
  Identifier(std::string value) : value(value){};

  std::string toString() override;
};

class Index : public Expression {
  Identifier *id;
  NumericLiteral *index;

 public:
  Index(Identifier *id, NumericLiteral *index) : id(id), index(index){};
  std::string toString() override;
};

class Slice : public Expression {
  Identifier *id;
  NumericLiteral *high_index;
  NumericLiteral *low_index;

 public:
  Slice(Identifier *id, NumericLiteral *high_index, NumericLiteral *low_index)
      : id(id), high_index(high_index), low_index(low_index){};
  std::string toString() override;
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
  Expression *left;
  Expression *right;

  BinOp::BinOp op;

 public:
  BinaryOp(Expression *left, BinOp::BinOp op, Expression *right)
      : left(left), op(op), right(right){};
  std::string toString() override;
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
};

class UnaryOp : public Expression {
  Expression *operand;

  UnOp::UnOp op;

 public:
  UnaryOp(Expression *operand, UnOp::UnOp op) : operand(operand), op(op){};
  std::string toString();
};

class TernaryOp : public Expression {
  Expression *cond;
  Expression *true_value;
  Expression *false_value;

 public:
  TernaryOp(Expression *cond, Expression *true_value, Expression *false_value)
      : cond(cond), true_value(true_value), false_value(false_value){};
  std::string toString();
};

class NegEdge : public Expression {
  Expression *value;

 public:
  NegEdge(Expression *value) : value(value){};
  std::string toString();
};

class PosEdge : public Expression {
  Expression *value;

 public:
  PosEdge(Expression *value) : value(value){};
  std::string toString();
};

class Port : public Node {
  // Required
  // `<name>` or `<name>[n]` or `name[n:m]`
  std::variant<Identifier *, Index *, Slice *> value;

  // technically the following are optional (e.g. port direction/data type
  // can be declared in the body of the definition), but for now let's force
  // users to declare ports in a single, unified way for
  // simplicity/maintenance

  // input | output | inout, we could make this an enum instead of string
  std::string direction;
  // probably can make these an enum too
  std::string data_type;

 public:
  std::string toString() { return "NOT IMPLEMENTED"; };
};

class Module : public Node {
  std::string name;
  std::vector<Port *> ports;
  std::vector<Node *> definition;
  std::map<std::string, NumericLiteral *> parameters;

 public:
  std::string toString() { return "NOT IMPLEMENTED"; };
};

class File : public Node {
  std::vector<Module *> modules;

 public:
  std::string toString() { return "NOT IMPLEMENTED"; };
};

class Statement : public Node {};

class BehavioralStatement : public Statement {};
class StructuralStatement : public Statement {};

class ModuleInstantiation : public StructuralStatement {
  std::string module_name;

  // TODO: For now we assume parameters are just numeric literals, are there
  // other types?
  std::map<std::string, NumericLiteral *> parameters;

  std::string instance_name;

  // map from instance port names to connection expression
  // NOTE: anonymous style of module connections is not supported
  std::map<std::string, std::variant<Identifier *, Index *, Slice *>>
      connections;

 public:
  std::string toString() { return "NOT IMPLEMENTED"; };
};

class Declaration : public Node {
  std::variant<Identifier, Index, Slice> value;
};

class Wire : Declaration {
 public:
  std::string toString() { return "NOT IMPLEMENTED"; };
};

class Reg : Declaration {
 public:
  std::string toString() { return "NOT IMPLEMENTED"; };
};

class ContinuousAssign : public StructuralStatement {
  std::variant<Identifier *, Index *, Slice *> target;
  Expression *value;

 public:
  std::string toString() { return "NOT IMPLEMENTED"; };
};

class BehavioralAssign : public BehavioralStatement {
  std::variant<Identifier *, Index *, Slice *> target;
  Expression *value;

 public:
  std::string toString() { return "NOT IMPLEMENTED"; };
};

class BlockingAssign : BehavioralAssign {
 public:
  std::string toString() { return "NOT IMPLEMENTED"; };
};

class NonBlockingAssign : BehavioralAssign {
 public:
  std::string toString() { return "NOT IMPLEMENTED"; };
};

class Always : public Node {
  std::vector<std::variant<Identifier *, PosEdge *, NegEdge *>>
      sensitivity_list;
  std::vector<std::variant<BehavioralStatement *, Declaration *>> body;

 public:
  std::string toString() { return "NOT IMPLEMENTED"; };
};

};  // namespace verilogAST
