#pragma once
#ifndef VERILOGAST_H
#define VERILOGAST_H

#include <algorithm>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>  // std::pair
#include <variant>
#include <vector>

namespace verilogAST {

template <typename T>
class WithComment : public T {
  std::string comment;

 public:
  WithComment(std::unique_ptr<T> node, std::string comment)
      : T(std::move(node)), comment(comment){};

  std::string toString() override {
    return T::toString() + "/*" + this->comment + "*/";
  };
  ~WithComment(){};
};

template <typename T>
std::unique_ptr<WithComment<T>> AddComment(std::unique_ptr<T> node,
                                           std::string comment) {
  return std::make_unique<WithComment<T>>(std::move(node), comment);
}

class Node {
 public:
  virtual std::string toString() = 0;
  virtual ~Node() = default;
};

class Expression : public Node {
 protected:
  virtual Expression* clone_impl() const = 0;

 public:
  virtual std::string toString() = 0;
  virtual ~Expression() = default;
  auto clone() const { return std::unique_ptr<Expression>(clone_impl()); }
};

enum Radix { BINARY, OCTAL, HEX, DECIMAL };

class NumericLiteral : public Expression {
 protected:
  virtual NumericLiteral* clone_impl() const override {
    return new NumericLiteral(this->value, this->size, this->_signed,
                              this->radix, this->always_codegen_size);
  };

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
  // always generate size prefix (default false will not codegen size for 32
  // bit literals)
  bool always_codegen_size = false;

  NumericLiteral(std::string value, unsigned int size, bool _signed,
                 Radix radix, bool always_codegen_size)
      : value(value),
        size(size),
        _signed(_signed),
        radix(radix),
        always_codegen_size(always_codegen_size){};

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
  auto clone() const { return std::unique_ptr<NumericLiteral>(clone_impl()); }
};

class Cast : public Expression {
 protected:
  virtual Cast* clone_impl() const override {
    return new Cast(this->width, this->expr->clone());
  };

 public:
  // integer width because we want to avoid numeric literals with ' like 2'b01
  unsigned int width;
  std::unique_ptr<Expression> expr;

  Cast(unsigned int width, std::unique_ptr<Expression> expr)
      : width(width), expr(std::move(expr)){};
  Cast(const Cast& rhs) : width(rhs.width), expr(expr->clone()){};
  auto clone() const { return std::unique_ptr<Cast>(clone_impl()); }

  std::string toString() override;
  ~Cast(){};
};

// TODO also need a string literal, as strings can be used as parameter values

class Identifier : public Expression {
 protected:
  virtual Identifier* clone_impl() const override {
    return new Identifier(*this);
  };

 public:
  std::string value;

  Identifier(std::string value);
  Identifier(const Identifier& rhs) : value(rhs.value){};
  auto clone() const { return std::unique_ptr<Identifier>(clone_impl()); }

  bool operator==(const Identifier& rhs) { return (this->value == rhs.value); }

  std::string toString() override;
  ~Identifier(){};
};

class Attribute : public Expression {
 protected:
  virtual Attribute* clone_impl() const override {
    return new Attribute(*this);
  };

 public:
  std::variant<std::unique_ptr<Identifier>, std::unique_ptr<Attribute>> value;
  std::string attr;

  Attribute(
      std::variant<std::unique_ptr<Identifier>, std::unique_ptr<Attribute>>
          value,
      std::string attr)
      : value(std::move(value)), attr(attr){};

  Attribute(const Attribute& rhs)
      : value(std::visit(
            [](auto&& value) -> std::variant<std::unique_ptr<Identifier>,
                                             std::unique_ptr<Attribute>> {
              return value->clone();
            },
            rhs.value)),
        attr(rhs.attr){};

  std::unique_ptr<Attribute> clone() const {
    return std::unique_ptr<Attribute>(clone_impl());
  }

  bool operator==(const Attribute& rhs) {
    return (this->value == rhs.value && this->attr == rhs.attr);
  }

  std::string toString() override;
  ~Attribute(){};
};

class String : public Expression {
 protected:
  virtual String* clone_impl() const override { return new String(*this); };

 public:
  std::string value;

  String(std::string value) : value(value){};
  String(const String& rhs) : value(rhs.value){};

  std::string toString() override;
  ~String(){};
  auto clone() const { return std::unique_ptr<String>(clone_impl()); }
};

class Slice : public Expression {
 protected:
  virtual Slice* clone_impl() const override {
    return new Slice(this->expr->clone(), this->high_index->clone(),
                     this->low_index->clone());
  };

 public:
  std::unique_ptr<Expression> expr;
  std::unique_ptr<Expression> high_index;
  std::unique_ptr<Expression> low_index;

  Slice(std::unique_ptr<Expression> expr,
        std::unique_ptr<Expression> high_index,
        std::unique_ptr<Expression> low_index)
      : expr(std::move(expr)),
        high_index(std::move(high_index)),
        low_index(std::move(low_index)){};
  Slice(const Slice& rhs)
      : expr(rhs.expr->clone()),
        high_index(rhs.high_index->clone()),
        low_index(rhs.low_index->clone()){};
  std::string toString() override;
  ~Slice(){};
  auto clone() const { return std::unique_ptr<Slice>(clone_impl()); }
};

class Index : public Expression {
 protected:
  virtual Index* clone_impl() const override {
    return new Index(this->clone_index_value(), this->index->clone());
  };

 public:
  std::variant<std::unique_ptr<Identifier>, std::unique_ptr<Attribute>,
               std::unique_ptr<Slice>, std::unique_ptr<Index>>
      value;
  std::unique_ptr<Expression> index;

  Index(std::variant<std::unique_ptr<Identifier>, std::unique_ptr<Attribute>,
                     std::unique_ptr<Slice>, std::unique_ptr<Index>>
            value,
        std::unique_ptr<Expression> index)
      : value(std::move(value)), index(std::move(index)){};

  Index(const Index& rhs)
      : value(rhs.clone_index_value()), index(rhs.index->clone()){};

  std::string toString() override;
  ~Index(){};
  auto clone() const { return std::unique_ptr<Index>(clone_impl()); }

 private:
  std::variant<std::unique_ptr<Identifier>, std::unique_ptr<Attribute>,
               std::unique_ptr<Slice>, std::unique_ptr<Index>>
  clone_index_value() const {
    return std::visit(
        [](auto&& value)
            -> std::variant<std::unique_ptr<Identifier>,
                            std::unique_ptr<Attribute>, std::unique_ptr<Slice>,
                            std::unique_ptr<Index>> { return value->clone(); },
        this->value);
  }
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
 protected:
  virtual BinaryOp* clone_impl() const override {
    return new BinaryOp(this->left->clone(), this->op, this->right->clone());
  };

 public:
  std::unique_ptr<Expression> left;
  BinOp::BinOp op;
  std::unique_ptr<Expression> right;

  BinaryOp(std::unique_ptr<Expression> left, BinOp::BinOp op,
           std::unique_ptr<Expression> right)
      : left(std::move(left)), op(op), right(std::move(right)){};
  BinaryOp(const BinaryOp& rhs)
      : left(rhs.left->clone()), op(rhs.op), right(rhs.right->clone()){};

  std::string toString() override;
  ~BinaryOp(){};
  auto clone() const { return std::unique_ptr<BinaryOp>(clone_impl()); }
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
 protected:
  virtual UnaryOp* clone_impl() const override {
    return new UnaryOp(this->operand->clone(), this->op);
  };

 public:
  std::unique_ptr<Expression> operand;

  UnOp::UnOp op;

  UnaryOp(std::unique_ptr<Expression> operand, UnOp::UnOp op)
      : operand(std::move(operand)), op(op){};
  UnaryOp(const UnaryOp& rhs) : operand(rhs.operand->clone()), op(rhs.op){};

  std::string toString() override;
  ~UnaryOp(){};
  auto clone() const { return std::unique_ptr<UnaryOp>(clone_impl()); }
};

class TernaryOp : public Expression {
 protected:
  virtual TernaryOp* clone_impl() const override {
    return new TernaryOp(this->cond->clone(), this->true_value->clone(),
                         this->false_value->clone());
  };

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
  TernaryOp(const TernaryOp& rhs)
      : cond(rhs.cond->clone()),
        true_value(rhs.true_value->clone()),
        false_value(rhs.false_value->clone()){};

  std::string toString() override;
  ~TernaryOp(){};
  auto clone() const { return std::unique_ptr<TernaryOp>(clone_impl()); }
};

class Concat : public Expression {
 protected:
  virtual Concat* clone_impl() const override {
    std::vector<std::unique_ptr<Expression>> new_args;
    for (const auto& arg : this->args) {
      new_args.push_back(arg->clone());
    }
    return new Concat(std::move(new_args));
  };

 public:
  std::vector<std::unique_ptr<Expression>> args;
  bool unpacked;

  Concat(std::vector<std::unique_ptr<Expression>> args, bool unpacked = false)
      : args(std::move(args)), unpacked(unpacked){};
  Concat(const Concat& rhs) {
    for (const auto& arg : rhs.args) args.push_back(arg->clone());
    this->unpacked = rhs.unpacked;
  };

  std::string toString() override;
  auto clone() const { return std::unique_ptr<Concat>(clone_impl()); }
};

class Replicate : public Expression {
 protected:
  virtual Replicate* clone_impl() const override {
    return new Replicate(this->num->clone(), this->value->clone());
  };

 public:
  std::unique_ptr<Expression> num;
  std::unique_ptr<Expression> value;

  Replicate(std::unique_ptr<Expression> num, std::unique_ptr<Expression> value)
      : num(std::move(num)), value(std::move(value)){};
  Replicate(const Replicate& rhs)
      : num(rhs.num->clone()), value(rhs.value->clone()){};

  std::string toString() override;
  auto clone() const { return std::unique_ptr<Replicate>(clone_impl()); }
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
  Call(std::string func) : func(func){};
  std::string toString();
  ~Call(){};
};

class CallExpr : public Expression, public Call {
 protected:
  virtual CallExpr* clone_impl() const override {
    std::vector<std::unique_ptr<Expression>> new_args;
    for (const auto& arg : this->args) {
      new_args.push_back(arg->clone());
    }
    return new CallExpr(this->func, std::move(new_args));
  };

 public:
  CallExpr(std::string func, std::vector<std::unique_ptr<Expression>> args)
      : Call(std::move(func), std::move(args)){};
  CallExpr(std::string func) : Call(std::move(func)){};
  CallExpr(const CallExpr& rhs) : Call(std::move(rhs.func)) {
    for (const auto& arg : rhs.args) {
      args.push_back(arg->clone());
    }
  };

  std::string toString() override { return Call::toString(); };
  auto clone() const { return std::unique_ptr<CallExpr>(clone_impl()); }
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

class NDVector : public Vector {
 public:
  std::vector<
      std::pair<std::unique_ptr<Expression>, std::unique_ptr<Expression>>>
      outer_dims;

  NDVector(std::unique_ptr<Identifier> id, std::unique_ptr<Expression> msb,
           std::unique_ptr<Expression> lsb,
           std::vector<std::pair<std::unique_ptr<Expression>,
                                 std::unique_ptr<Expression>>>
               outer_dims)
      : Vector(std::move(id), std::move(msb), std::move(lsb)),
        outer_dims(std::move(outer_dims)){};
  std::string toString() override;
  ~NDVector(){};
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
      : value(std::move(value)), direction(direction), data_type(data_type){};
  Port(std::unique_ptr<Port> port)
      : value(std::move(port->value)),
        direction(port->direction),
        data_type(port->data_type){};
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

class BehavioralStatement : public Statement {};
class StructuralStatement : public Statement {};

class SingleLineComment : public StructuralStatement,
                          public BehavioralStatement {
 public:
  std::string value;
  std::unique_ptr<Statement> statement;  // optional

  SingleLineComment(std::string value)
      : value(value), statement(std::unique_ptr<Statement>{}){};
  SingleLineComment(std::string value, std::unique_ptr<Statement> statement)
      : value(value), statement(std::move(statement)){};
  std::string toString();
  ~SingleLineComment(){};
};

class BlockComment : public StructuralStatement, public BehavioralStatement {
 public:
  std::string value;

  BlockComment(std::string value) : value(value){};
  std::string toString() { return "/*\n" + value + "\n*/"; };
  ~BlockComment(){};
};

class InlineVerilog : public StructuralStatement {
  // Serializes into `value`, so allows the inclusion of arbitrary verilog
  // statement(s) in the body of a module definition.  The contents of
  // `value` must be a valid verilog statement inside a module body.  The
  // contents are not validated.
 public:
  std::string value;

  InlineVerilog(std::string value) : value(value){};
  std::string toString() { return value; };
  ~InlineVerilog(){};
};

typedef std::vector<std::pair<
    std::variant<std::unique_ptr<Identifier>, std::unique_ptr<Vector>>,
    std::unique_ptr<Expression>>>
    Parameters;

typedef std::vector<std::pair<std::string, std::unique_ptr<Expression>>>
    ConnectionVector;

class Connections {
 public:
  Connections() : connections() {}
  ~Connections() = default;

  // Non-copyable class
  Connections(const Connections&) = delete;

  // Takes ownership of @expr.
  void insert(std::string name, std::unique_ptr<Expression> expr) {
    connections.push_back(std::make_pair(name, std::move(expr)));
  }

  // Releases ownership of expression at @name if exists, othwerwise throws
  // error.
  std::unique_ptr<Expression> at(std::string name) {
    auto is_name = [name](auto& element) { return element.first == name; };
    auto it = std::find_if(connections.begin(), connections.end(), is_name);
    if (it != connections.end()) return std::move(it->second);
    throw std::runtime_error("Could not find '" + name + "'");
  }

  ConnectionVector::iterator begin() { return connections.begin(); }
  ConnectionVector::iterator end() { return connections.end(); }

  bool empty() const { return connections.empty(); }

 private:
  ConnectionVector connections;
};

class ModuleInstantiation : public StructuralStatement {
 public:
  std::string module_name;

  // parameter,value
  Parameters parameters;

  std::string instance_name;

  // NOTE: anonymous style of module connections is not supported
  std::unique_ptr<Connections> connections;

  // TODO Need to make sure that the instance parameters are a subset of the
  // module parameters
  ModuleInstantiation(std::string module_name, Parameters parameters,
                      std::string instance_name,
                      std::unique_ptr<Connections> connections)
      : module_name(module_name),
        parameters(std::move(parameters)),
        instance_name(instance_name),
        connections(std::move(connections)){};
  std::string toString();
  ~ModuleInstantiation(){};
};

class IfMacro : public StructuralStatement {
  virtual std::string getMacroString() = 0;
  public:
   std::string condition_str;
   std::vector<std::unique_ptr<StructuralStatement>> body;
   IfMacro(std::string condition_str,
           std::vector<std::unique_ptr<StructuralStatement>> body)
       : condition_str(condition_str), body(std::move(body)){};
   ~IfMacro(){};
   std::string toString();
};

class IfDef : public IfMacro {
  std::string getMacroString() { return "`ifdef "; };
    public:
   IfDef(std::string condition_str,
           std::vector<std::unique_ptr<StructuralStatement>> body)
       : IfMacro(condition_str, std::move(body)){};
   ~IfDef(){};
};

class IfNDef : public IfMacro {
  std::string getMacroString() { return "`ifndef "; };
    public:
   IfNDef(std::string condition_str,
           std::vector<std::unique_ptr<StructuralStatement>> body)
       : IfMacro(condition_str, std::move(body)){};
   ~IfNDef(){};
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
  CallStmt(std::string func) : Call(std::move(func)){};
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
  std::vector<std::unique_ptr<BehavioralStatement>> body;

  Always(std::vector<
             std::variant<std::unique_ptr<Identifier>, std::unique_ptr<PosEdge>,
                          std::unique_ptr<NegEdge>, std::unique_ptr<Star>>>
             sensitivity_list,
         std::vector<std::unique_ptr<BehavioralStatement>> body)
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

class If : public BehavioralStatement {
 public:
  std::unique_ptr<Expression> cond;
  std::vector<std::unique_ptr<BehavioralStatement>> true_body;
  // pair of cond + body for else if cases
  std::vector<std::pair<std::unique_ptr<Expression>,
                        std::vector<std::unique_ptr<BehavioralStatement>>>>
      else_ifs;
  std::vector<std::unique_ptr<BehavioralStatement>> else_body;

  If(std::unique_ptr<Expression> cond,
     std::vector<std::unique_ptr<BehavioralStatement>> true_body,
     std::vector<std::pair<std::unique_ptr<Expression>,
                           std::vector<std::unique_ptr<BehavioralStatement>>>>
         else_ifs,
     std::vector<std::unique_ptr<BehavioralStatement>> else_body)
      : cond(std::move(cond)),
        true_body(std::move(true_body)),
        else_ifs(std::move(else_ifs)),
        else_body(std::move(else_body)){};

  If(std::unique_ptr<Expression> cond,
     std::vector<std::unique_ptr<BehavioralStatement>> true_body,
     std::vector<std::unique_ptr<BehavioralStatement>> else_body)
      : cond(std::move(cond)),
        true_body(std::move(true_body)),
        else_body(std::move(else_body)){};

  std::string toString();
  ~If(){};
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

  Module(std::string name, std::vector<std::unique_ptr<AbstractPort>> ports,
         std::vector<std::variant<std::unique_ptr<StructuralStatement>,
                                  std::unique_ptr<Declaration>>>
             body)
      : name(name), ports(std::move(ports)), body(std::move(body)){};

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

}  // namespace verilogAST
#endif
