#include "benscope/parsing/parser.h"
#include "benscope/parsing/printer.h"

#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "benscope/parsing/ast.h"
#include "benscope/parsing/lexer.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace benscope {
namespace {

using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::NotNull;

std::unique_ptr<AST> Parse(std::string_view text) {
  std::stringstream ss;
  ss << text;
  return Parser(std::make_unique<Lexer>(&ss)).ParseNext();
}

std::unique_ptr<AST> ParseExpr(std::string_view text) {
  std::stringstream ss;
  ss << text;
  return Parser(std::make_unique<Lexer>(&ss)).ParseExpression();
}

TEST(ParserTest, CallExpr) {
  auto ast = ParseExpr("(f x y)");

  PrintingVisitor v;
  ast->Accept(v);
  EXPECT_THAT(v.ToString(), Eq("[CALL f {x} {y}]"));

  auto call = dynamic_cast<CallExprAST *>(ast.get());
  ASSERT_THAT(call, NotNull());
  EXPECT_THAT(call->callee, Eq("f"));

  std::vector<std::string> vars;
  for (auto &arg : call->args) {
    auto var = dynamic_cast<VariableExprAST *>(arg.get());
    EXPECT_THAT(var, NotNull());
    if (var)
      vars.push_back(var->name_);
  }

  EXPECT_THAT(vars, ElementsAre("x", "y"));
}

TEST(ParserTest, MultiLine) {
  auto ast = Parse(R"(
    (def fib (old new gen)
      (if
        (< gen 2)
        new
        (fib new (+ old new) (- gen 1))
      )
    )
  )");

  PrintingVisitor v;
  ast->Accept(v);

  EXPECT_THAT(v.ToString(),
              Eq("[DEFINE fib(old new gen) :== [IF [{gen} < [2]] THEN {new} "
                 "ELSE [CALL fib {new} [{old} + {new}] [{gen} - [1]]]]]"));
}

} // namespace
} // namespace benscope