#include <iostream>
#include <memory>
#include <sstream>
#include <utility>

#include "absl/strings/str_join.h"
#include "benscope/parsing/ast.h"
#include "benscope/parsing/lexer.h"
#include "benscope/parsing/parser.h"

namespace benscope {
namespace {

class PrintingVisitor : public AstVisitor {
public:
  void Visit(const BinaryExprAST &expr) override {
    std::cerr << "[";
    expr.lhs->Accept(*this);
    std::cerr << " ";
    std::cerr.put(expr.op);
    std::cerr << " ";
    expr.rhs->Accept(*this);
    std::cerr << "]";
  }

  void Visit(const CallExprAST &expr) override {
    std::cerr << "[CALL " << expr.callee;
    for (const auto &arg : expr.args) {
      std::cerr << " ";
      arg->Accept(*this);
    }
    std::cerr << "]";
  }

  void Visit(const IfExprAST &expr) override {
    std::cerr << "[IF [";
    expr.test->Accept(*this);
    std::cerr << "] THEN [";
    expr.if_true->Accept(*this);
    std::cerr << "] ELSE [";
    expr.if_false->Accept(*this);
    std::cerr << "]]";
  }

  void Visit(const NumberExprAST &expr) override {
    std::cerr << "[" << expr.val << "]";
  };

  void Visit(const VariableExprAST &expr) override {
    std::cerr << "{" << expr.name_ << "}";
  };

  void Visit(const FunctionAST &ast) override {
    inFunc = true;
    std::cerr << "[DEFINE ";
    ast.proto->Accept(*this);
    std::cerr << " :== ";
    ast.body->Accept(*this);
    std::cerr << "]\n";
    inFunc = false;
  };

  void Visit(const PrototypeAST &ast) override{
    if (!inFunc) std::cerr << "[EXTERN ";
    std::cerr << ast.name << "(" << absl::StrJoin(ast.args, " ") << ")";
    if (!inFunc) std::cerr << "]\n";
  };

private:
  bool inFunc = false;
};

void MainLoop(Parser *parser) {
  PrintingVisitor printer;
  while (!parser->eof()) {
    std::unique_ptr<AST> ast = parser->ParseNext();
    if (ast) {
      std::cerr << "\n";
      ast->Accept(printer);
    } else {
      std::cerr << "Unhandled text.";
      parser->GetNextToken();
    }
  }
}

} // namespace
} // namespace benscope

int main() {
  while (!std::cin.eof()) {
    std::stringstream input;
    std::string line;
    std::cerr << "\nready> ";
    std::getline(std::cin, line);
    // std::cerr << "Line {" << line << "}\n";
    input << line;
    auto lexer = std::make_unique<benscope::Lexer>(&input);
    benscope::Parser parser(std::move(lexer));
    benscope::MainLoop(&parser);
  }

  return 0;
}