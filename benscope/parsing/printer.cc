#include <string>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"

#include "benscope/parsing/ast.h"
#include "benscope/parsing/printer.h"

namespace benscope {

void PrintingVisitor::Visit(const BinaryExprAST &expr) {
  absl::StrAppend(&_buffer, "[");
  expr.lhs->Accept(*this);
  _buffer.push_back(' ');
  _buffer.push_back(expr.op);
  _buffer.push_back(' ');
  expr.rhs->Accept(*this);
  absl::StrAppend(&_buffer, "]");
}

void PrintingVisitor::Visit(const CallExprAST &expr) {
  absl::StrAppend(&_buffer, "[CALL ", expr.callee);
  for (const auto &arg : expr.args) {
    absl::StrAppend(&_buffer, " ");
    arg->Accept(*this);
  }
  absl::StrAppend(&_buffer, "]");
}

void PrintingVisitor::Visit(const IfExprAST &expr) {
  absl::StrAppend(&_buffer, "[IF ");
  expr.test->Accept(*this);
  absl::StrAppend(&_buffer, " THEN ");
  expr.if_true->Accept(*this);
  absl::StrAppend(&_buffer, " ELSE ");
  expr.if_false->Accept(*this);
  absl::StrAppend(&_buffer, "]");
}

void PrintingVisitor::Visit(const NumberExprAST &expr) {
  absl::StrAppend(&_buffer, "[", expr.val, "]");
};

void PrintingVisitor::Visit(const VariableExprAST &expr) {
  absl::StrAppend(&_buffer, "{", expr.name_, "}");
};

void PrintingVisitor::Visit(const FunctionAST &ast) {
  _inFunc = true;
  absl::StrAppend(&_buffer, "[DEFINE ");
  ast.proto->Accept(*this);
  absl::StrAppend(&_buffer, " :== ");
  ast.body->Accept(*this);
  absl::StrAppend(&_buffer, "]");
  _inFunc = false;
};

void PrintingVisitor::Visit(const PrototypeAST &ast) {
  if (!_inFunc)
    absl::StrAppend(&_buffer, "[EXTERN ");
  absl::StrAppend(&_buffer, ast.name, "(", absl::StrJoin(ast.args, " "), ")");
  if (!_inFunc)
    absl::StrAppend(&_buffer, "]");
};

std::string_view PrintingVisitor::ToStringView() { return _buffer; }
const std::string &PrintingVisitor::ToString() { return _buffer; }

} // namespace benscope
