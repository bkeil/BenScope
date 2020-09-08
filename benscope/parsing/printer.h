// Renders a flat string from the AST.

#ifndef __BENSCOPE_PARSING_PRINTER_H__
#define __BENSCOPE_PARSING_PRINTER_H__

#include <string>

#include "benscope/parsing/ast.h"

namespace benscope {
class PrintingVisitor : public AstVisitor {
public:
  void Visit(const BinaryExprAST &expr) override;
  void Visit(const CallExprAST &expr) override;
  void Visit(const IfExprAST &expr) override;
  void Visit(const NumberExprAST &expr) override;
  void Visit(const VariableExprAST &expr) override;

  void Visit(const FunctionAST &ast) override;
  void Visit(const PrototypeAST &ast) override;

  std::string_view ToStringView();
  const std::string &ToString();

private:
  std::string _buffer;
  bool _inFunc = false;
};
} // namespace benscope

#endif // __BENSCOPE_PARSING_PRINTER_H__