#include "benscope/parsing/ast.h"

namespace benscope {

void NumberExprAST::Accept(AstVisitor &visitor) const { visitor.Visit(*this); }
void VariableExprAST::Accept(AstVisitor &visitor) const {
  visitor.Visit(*this);
}
void BinaryExprAST::Accept(AstVisitor &visitor) const { visitor.Visit(*this); }
void CallExprAST::Accept(AstVisitor &visitor) const { visitor.Visit(*this); }
void IfExprAST::Accept(AstVisitor &visitor) const { visitor.Visit(*this); }

void FunctionAST::Accept(AstVisitor &visitor) const { visitor.Visit(*this); }
void PrototypeAST::Accept(AstVisitor &visitor) const { visitor.Visit(*this); }

} // namespace benscope