#include <map>
#include <string>
#include <utility> // std::pair
#include <variant>
#include <vector>

namespace verilogAST {

class Node {};

class Expression : Node {};

class NumericLiteral : Expression {
  /// For now, we model values as strings because it depends on their radix
  // (alternatively, we could store an unsigned integer representation and
  //  convert it during code generation)
  std::string value;
  unsigned int size;
  bool _signed; // default false
  // TODO: This could be an enum
  std::string radix; // default decimal ('d)
};

class Identifier : Expression {
  std::string value;

public:
  Identifier(std::string value) : value(value){};
};

class Index : Expression {
  Identifier *id;
  NumericLiteral *index;
};

class Slice : Expression {
  Identifier *id;
  NumericLiteral *high_index;
  NumericLiteral *low_index;
};

class BinaryOp : Expression {
  Expression *left;
  Expression *right;

  // TODO: Enum of support ops
  std::string op;
};

class UnaryOp : Expression {
  Expression *operand;

  // TODO: Enum of support ops
  std::string op;
};

class TernaryOp : Expression {
  Expression *cond;
  Expression *true_value;
  Expression *false_value;
};

class NegEdge : Expression {
  Expression *value;
};

class PosEdge : Expression {
  Expression *value;
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
};

class Module : Node {
  std::string name;
  std::vector<Port *> ports;
  std::vector<Node *> definition;
  std::map<std::string, NumericLiteral *> parameters;
};

class File : Node {
  std::vector<Module *> modules;
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
};

class Declaration : Node {
  std::variant<Identifier, Index, Slice> value;
};

class Wire : Declaration {};

class Reg : Declaration {};

class ContinuousAssign : StructuralStatement {
  std::variant<Identifier *, Index *, Slice *> target;
  Expression *value;
};

class BehavioralAssign : BehavioralStatement {
  std::variant<Identifier *, Index *, Slice *> target;
  Expression *value;
};

class BlockingAssign : BehavioralAssign {};
class NonBlockingAssign : BehavioralAssign {};

class Always : Node {
  std::vector<std::variant<Identifier *, PosEdge *, NegEdge *>>
      sensitivity_list;
  std::vector<std::variant<BehavioralStatement *, Declaration *>> body;
};

}; // namespace verilogAST
