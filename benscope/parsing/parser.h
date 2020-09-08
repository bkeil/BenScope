#ifndef __BENSCOPE_PARSING_PARSER_H__
#define __BENSCOPE_PARSING_PARSER_H__

#include <memory>
#include <utility>
#include <vector>

#include "benscope/parsing/ast.h"
#include "benscope/parsing/lexer.h"

namespace benscope {

class Parser {
public:
  explicit Parser(std::unique_ptr<Lexer> lexer)
      : lexer_(std::move(lexer)), token_(lexer_->token()) {}

  std::unique_ptr<AST> ParseNext();
  std::unique_ptr<ExprAST> ParseExpression();

  const Token &GetNextToken();

  bool eof();

private:

  std::unique_ptr<AST> ParseParenExpr();


  std::unique_ptr<FunctionAST> ParseDefinition();
  std::unique_ptr<PrototypeAST> ParseExtern();
  std::unique_ptr<PrototypeAST> ParsePrototype();

  std::unique_ptr<NumberExprAST> ParseNumberExpr();
  std::unique_ptr<VariableExprAST> ParseVariableExpr();

  std::unique_ptr<CallExprAST> ParseCallExpr();
  std::unique_ptr<IfExprAST> ParseIfExpr();
  std::unique_ptr<BinaryExprAST> ParseOpExpr();

  std::unique_ptr<Lexer> lexer_;
  const Token &token_;
};
} // namespace benscope

#endif // __BENSCOPE_PARSING_PARSER_H__