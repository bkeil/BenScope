#include <iostream>
#include <memory>
#include <utility>

#include "benscope/c64/codegen.h"
#include "benscope/parsing/ast.h"
#include "benscope/parsing/lexer.h"
#include "benscope/parsing/parser.h"

int main(int argc, char *argv[]) {
  auto lexer = std::make_unique<benscope::Lexer>(&std::cin);
  benscope::Parser parser(std::move(lexer));

  benscope::c64::CodeGen codegen;
  while (!parser.eof()) {
    std::unique_ptr<benscope::AST> ast = parser.ParseNext();
    if (ast) {
      ast->Accept(codegen);
    } else {
      std::cerr << "Unhandled text.";
      parser.GetNextToken();
    }
  }
  std::cout << codegen.ToString();
}