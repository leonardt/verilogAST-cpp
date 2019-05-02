#include <map>
#include <string>
#include <utility> // std::pair
#include <variant>
#include <vector>

namespace verilogAST {

class Node {
public:
  virtual std::string toString() = 0;
};

class Expression : Node {};

enum Radix { binary, octal, hex, decimal };

class NumericLiteral : Expression {
  /// For now, we model values as strings because it depends on their radix
  // (alternatively, we could store an unsigned integer representation and
  //  convert it during code generation)
  //
  // TODO Maybe add special toString logic for the default case? E.g. if we're
  // generating a 32 bit unsigned decimal literal (commonly used for indexing
  // into ports) then we don't need to generate the "32'd" prefix
  std::string value;
  unsigned int size; // default 32
  bool _signed;      // default false
  Radix radix;       // default decimal

public:
  NumericLiteral(std::string value, unsigned int size, bool _signed,
                 Radix radix)
      : value(value), size(size), _signed(_signed), radix(radix){};

  NumericLiteral(std::string value, unsigned int size, bool _signed)
      : value(value), size(size), _signed(_signed), radix(decimal){};

  NumericLiteral(std::string value, unsigned int size)
      : value(value), size(size), _signed(false), radix(decimal){};

  NumericLiteral(std::string value)
      : value(value), size(32), _signed(false), radix(decimal){};
  std::string toString();
};

class Identifier : Expression {
  std::string value;

public:
  Identifier(std::string value) : value(value){};
  std::string toString();
};

class Index : Expression {
  Identifier *id;
  NumericLiteral *index;

public:
  Index(Identifier *id, NumericLiteral *index) : id(id), index(index){};
  std::string toString();
};

class Slice : Expression {
  Identifier *id;
  NumericLiteral *high_index;
  NumericLiteral *low_index;

public:
  Slice(Identifier *id, NumericLiteral *high_index, NumericLiteral *low_index)
      : id(id), high_index(high_index), low_index(low_index){};
  std::string toString();
};

class BinaryOp : Expression {
  Expression *left;
  Expression *right;

  // TODO: Enum of support ops
  std::string op;

public:
  std::string toString() { return "NOT IMPLEMENTED"; };
};

class UnaryOp : Expression {
  Expression *operand;

  // TODO: Enum of support ops
  std::string op;

public:
  std::string toString() { return "NOT IMPLEMENTED"; };
};

class TernaryOp : Expression {
  Expression *cond;
  Expression *true_value;
  Expression *false_value;

public:
  std::string toString() { return "NOT IMPLEMENTED"; };
};

class NegEdge : Expression {
  Expression *value;

public:
  std::string toString() { return "NOT IMPLEMENTED"; };
};

class PosEdge : Expression {
  Expression *value;

public:
  std::string toString() { return "NOT IMPLEMENTED"; };
};

class Port : Node {
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

class Module : Node {
  std::string name;
  std::vector<Port *> ports;
  std::vector<Node *> definition;
  std::map<std::string, NumericLiteral *> parameters;

public:
  std::string toString() { return "NOT IMPLEMENTED"; };
};

class File : Node {
  std::vector<Module *> modules;

public:
  std::string toString() { return "NOT IMPLEMENTED"; };
};

class Statement : Node {};

class BehavioralStatement : Node {};
class StructuralStatement : Node {};

class ModuleInstantiation : StructuralStatement {
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

class Declaration : Node {
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

class ContinuousAssign : StructuralStatement {
  std::variant<Identifier *, Index *, Slice *> target;
  Expression *value;

public:
  std::string toString() { return "NOT IMPLEMENTED"; };
};

class BehavioralAssign : BehavioralStatement {
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

class Always : Node {
  std::vector<std::variant<Identifier *, PosEdge *, NegEdge *>>
      sensitivity_list;
  std::vector<std::variant<BehavioralStatement *, Declaration *>> body;

public:
  std::string toString() { return "NOT IMPLEMENTED"; };
};

}; // namespace verilogAST
