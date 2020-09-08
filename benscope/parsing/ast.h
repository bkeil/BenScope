#ifndef __BENSCOPE_PARSING_AST_H__
#define __BENSCOPE_PARSING_AST_H__

#include <memory>
#include <string>
#include <vector>

namespace benscope {

class AstVisitor;

static constexpr std::string_view kAnonExpr = "__anon_expr";

struct AST {
  virtual ~AST() {}
  virtual void Accept(AstVisitor &visitor) const = 0;
};

struct ExprAST : public AST {};

struct NumberExprAST : public ExprAST {
  explicit NumberExprAST(double value) : val(value) {}
  void Accept(AstVisitor &visitor) const override;

  double val;
};

struct VariableExprAST : public ExprAST {
  VariableExprAST(std::string_view name) : name_(name) {}
  void Accept(AstVisitor &visitor) const override;

  std::string name_;
};

struct BinaryExprAST : public ExprAST {
  BinaryExprAST(char operation, std::unique_ptr<ExprAST> leftHS,
                std::unique_ptr<ExprAST> rightHS)
      : op(operation), lhs(std::move(leftHS)), rhs(std::move(rightHS)) {}
  void Accept(AstVisitor &visitor) const override;

  char op;
  std::unique_ptr<ExprAST> lhs, rhs;
};

struct CallExprAST : public ExprAST {
  CallExprAST(const std::string &Callee,
              std::vector<std::unique_ptr<ExprAST>> Args)
      : callee(Callee), args(std::move(Args)) {}
  void Accept(AstVisitor &visitor) const override;

  std::string callee;
  std::vector<std::unique_ptr<ExprAST>> args;
};

struct IfExprAST : public ExprAST {
  IfExprAST(std::unique_ptr<ExprAST> Test, std::unique_ptr<ExprAST> If_true,
            std::unique_ptr<ExprAST> If_false)
      : test(std::move(Test)), if_true(std::move(If_true)),
        if_false(std::move(If_false)) {}
  void Accept(AstVisitor &visitor) const override;

  std::unique_ptr<ExprAST> test, if_true, if_false;
};

/// PrototypeAST - This class represents the "prototype" for a function,
/// which captures its name, and its argument names (thus implicitly the number
/// of arguments the function takes).
struct PrototypeAST : public AST {
  PrototypeAST() = default;
  PrototypeAST(const std::string &name, std::vector<std::string> Args)
      : name(name), args(std::move(Args)) {}
  void Accept(AstVisitor &visitor) const override;

  std::string name;
  std::vector<std::string> args;
};

/// FunctionAST - This class represents a function definition itself.
struct FunctionAST : public AST {
public:
  FunctionAST(std::unique_ptr<PrototypeAST> Proto,
              std::unique_ptr<ExprAST> Body)
      : proto(std::move(Proto)), body(std::move(Body)) {}
  void Accept(AstVisitor &visitor) const override;

  std::unique_ptr<PrototypeAST> proto;
  std::unique_ptr<ExprAST> body;
};

class AstVisitor {
public:
  virtual void Visit(const BinaryExprAST &expr) = 0;
  virtual void Visit(const CallExprAST &expr) = 0;
  virtual void Visit(const IfExprAST &expr) = 0;
  virtual void Visit(const NumberExprAST &expr) = 0;
  virtual void Visit(const VariableExprAST &expr) = 0;

  virtual void Visit(const FunctionAST &ast) = 0;
  virtual void Visit(const PrototypeAST &ast) = 0;
};

} // namespace benscope
#endif // __BENSCOPE_PARSING_AST_H__