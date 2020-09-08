#include <iostream>

#include "benscope/parsing/parser.h"

namespace benscope {
namespace {
template <typename T> std::unique_ptr<T> LogError(std::string_view message) {
  std::cerr << message << "\n";
  return nullptr;
}
} // namespace

bool Parser::eof() { return token_.type == Token::kEof; }

std::unique_ptr<AST> Parser::ParseNext() {
  //std::cerr << "\nParseNext\n";
  if (token_.type != '(')
    return LogError<ExprAST>(
        "Expected '(' at the beginning of a top level expression.");

  GetNextToken(); // Eat '('

  auto ast = ParseParenExpr();

  if (dynamic_cast<ExprAST *>(ast.get())) {
    std::unique_ptr<ExprAST> expr(dynamic_cast<ExprAST *>(ast.release()));
    // Make an anonymous proto.
    auto p = std::make_unique<PrototypeAST>(std::string(kAnonExpr),
                                            std::vector<std::string>{});
    return std::make_unique<FunctionAST>(std::move(p), std::move(expr));
  }

  return ast;
}

const Token &Parser::GetNextToken() {
  lexer_->Next();
  //lexer_->Debug();
  return token_;
}

/// definition ::= 'def' prototype expression
std::unique_ptr<FunctionAST> Parser::ParseDefinition() {
  //std::cerr << "\nParseDefinition\n";
  if (token_.type != Token::kDef) {
    std::cerr << "Weird... this definition starts on an unexpected token.\n";
  }
  GetNextToken(); // eat def.

  auto p = ParsePrototype();
  if (!p)
    return LogError<FunctionAST>("Error in prototype of definition.");

  auto e = ParseExpression();
  if (!e)
    return LogError<FunctionAST>("Error in expression of definition.");

  if (token_.type != ')')
    return LogError<FunctionAST>("Expected ')' at end of definition.");

  GetNextToken();
  return std::make_unique<FunctionAST>(std::move(p), std::move(e));
}

std::unique_ptr<ExprAST> Parser::ParseExpression() {
  //std::cerr << "\nParseExpression\n";
  switch (token_.type) {
  case Token::kNumber:
    return ParseNumberExpr();
  case Token::kIdentifier:
    return ParseVariableExpr();
  case '(': {
    GetNextToken();
    auto ast = ParseParenExpr();
    if (dynamic_cast<ExprAST *>(ast.get())) {
      return std::unique_ptr<ExprAST>(dynamic_cast<ExprAST *>(ast.release()));
    }
    return LogError<ExprAST>("Found an internal extern or def.");
  }
  case Token::kEof:
    return LogError<ExprAST>("Unexpected EOF.");
  default:
    return LogError<ExprAST>("Unexpected token at expression beginning.");
  }
}

std::unique_ptr<PrototypeAST> Parser::ParseExtern() {
  //std::cerr << "\nParseExtern\n";
  GetNextToken(); // eat 'extern'
  auto p = ParsePrototype();
  if (token_.type != ')')
    return LogError<PrototypeAST>("Expected ')' at end of extern declaration.");
  GetNextToken();
  return std::move(p);
}

/// prototype
///   ::= id '(' id* ')'
std::unique_ptr<PrototypeAST> Parser::ParsePrototype() {
  //std::cerr << "\nParsePrototype)\n";
  if (token_.type != Token::kIdentifier)
    return LogError<PrototypeAST>(
        "Expected function name at start of prototype");

  std::string fnName(token_.value.string_value);
  GetNextToken();

  if (token_.type != '(')
    return LogError<PrototypeAST>("Expected '(' in prototype");

  std::vector<std::string> argNames;
  while (GetNextToken().type == Token::kIdentifier) {
    argNames.emplace_back(token_.value.string_value);
  }

  if (token_.type != ')')
    return LogError<PrototypeAST>("Expected ')' at end of prototype");

  // success.
  GetNextToken(); // eat ')'.

  // std::cerr << "(Leave ParsePrototype)\n";
  return std::make_unique<PrototypeAST>(fnName, std::move(argNames));
}

/// numberexpr ::= number
std::unique_ptr<NumberExprAST> Parser::ParseNumberExpr() {
  //std::cerr << "\nParseNumberExpr\n";
  auto result =
      std::make_unique<NumberExprAST>(lexer_->token().value.double_value);
  GetNextToken(); // consume the number
  return std::move(result);
}

/// variableexpr ::= identifier
std::unique_ptr<VariableExprAST> Parser::ParseVariableExpr() {
  //std::cerr << "\nParseVariableExpr\n";
  auto result = std::make_unique<VariableExprAST>(token_.value.string_value);
  GetNextToken(); // consume the identifier
  return std::move(result);
}

std::unique_ptr<IfExprAST> Parser::ParseIfExpr() {
  //std::cerr << "\nParseIfExpr\n";
  if (token_.type != Token::kIf) {
    std::cerr
        << "Very strange... this 'if' doesn't start with an [if] token.\n";
  }
  GetNextToken(); // Eat 'if'

  //std::cerr << "\nParseIfExpr -- Test\n";
  auto test = ParseExpression();
  if (!test)
    return LogError<IfExprAST>("Error parsing test expression.");

  //std::cerr << "\nParseIfExpr -- True\n";
  auto if_true = ParseExpression();
  if (!if_true)
    return LogError<IfExprAST>("Error parsing if-true expression.");

  //std::cerr << "\nParseIfExpr -- False\n";
  auto if_false = ParseExpression();
  if (!if_false)
    return LogError<IfExprAST>("Error parsing if-false expression.");

  if (token_.type != ')')
    return LogError<IfExprAST>("Missing ')' at end of if-expression.");

  GetNextToken(); // Eat ')'.

  return std::make_unique<IfExprAST>(std::move(test), std::move(if_true),
                                     std::move(if_false));
}

std::unique_ptr<AST> Parser::ParseParenExpr() {
  //std::cerr << "\nParseParenExpr\n";
  switch (token_.type) {
  case Token::kIdentifier:
    return ParseCallExpr();
  case Token::kNumber:
    return LogError<ExprAST>(
        "Found number at the beginning of a parenthetical.");
  case Token::kIf:
    return ParseIfExpr();
  case Token::kDef:
    return ParseDefinition();
  case Token::kExtern:
    return ParseExtern();
  case Token::kEof:
    return LogError<ExprAST>("EOF found while inside a parenthetical.");
  default:
    return ParseOpExpr();
  }
}

/// callexpr ::= {'('} identifier expr* ')'
std::unique_ptr<CallExprAST> Parser::ParseCallExpr() {
  // std::cerr << "(Enter ParseCallExpr)\n";
  std::string idName(token_.value.string_value);

  std::vector<std::unique_ptr<ExprAST>> args;
  GetNextToken(); // Eat callee name;
  while (token_.type != ')' && token_.type != Token::kEof) {
    // std::cerr << "(Start arg)";
    args.push_back(ParseExpression());
    // std::cerr << "(End arg)";
  }

  if (token_.type != ')') {
    // std::cerr << "(Leave ParseCallExpr)\n";
    return LogError<CallExprAST>("Expected ')' while parsing call.");
  }

  GetNextToken(); // eat ')'
  // std::cerr << "(Leave ParseCallExpr)\n";
  return std::make_unique<CallExprAST>(idName, std::move(args));
}

std::unique_ptr<BinaryExprAST> Parser::ParseOpExpr() {
  char op = token_.type;
  //std::cerr << "\nParseOpExpr (op = '" << op << " / " << (int)op << "')\n";
  GetNextToken();

  auto lhs = ParseExpression();
  if (!lhs)
    return LogError<BinaryExprAST>("Error in first argument.");

  auto rhs = ParseExpression();
  if (!rhs)
    return LogError<BinaryExprAST>("Error in second argument.");

  if (token_.type != ')')
    return LogError<BinaryExprAST>("Missing ')' after binary expression.");

  GetNextToken(); // Eat ')'.
  return std::make_unique<BinaryExprAST>(op, std::move(lhs), std::move(rhs));
}

} // namespace benscope