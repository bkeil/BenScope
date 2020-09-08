#include "benscope/parsing/lexer.h"

#include <sstream>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace benscope {
namespace {

using ::testing::Eq;

TEST(LexerTest, ReportsEOF) {
  std::stringstream ss;
  Lexer lexer(&ss);

  EXPECT_THAT(lexer.token().type, Eq(Token::kEof));
  lexer.Next();
  EXPECT_THAT(lexer.token().type, Eq(Token::kEof));
}

TEST(LexerTest, NonWhitespaceDelimiters) {
  std::stringstream ss;
  ss << R"(();"'`|)";

  Lexer lexer(&ss);

  EXPECT_THAT(lexer.token().type, Eq('('));
  lexer.Next();
  EXPECT_THAT(lexer.token().type, Eq(')'));
  lexer.Next();
  EXPECT_THAT(lexer.token().type, Eq(';'));
  lexer.Next();
  EXPECT_THAT(lexer.token().type, Eq('"'));
  lexer.Next();
  EXPECT_THAT(lexer.token().type, Eq('\''));
  lexer.Next();
  EXPECT_THAT(lexer.token().type, Eq('`'));
  lexer.Next();
  EXPECT_THAT(lexer.token().type, Eq('|'));
  lexer.Next();
  EXPECT_THAT(lexer.token().type, Eq(Token::kEof));
}

TEST(LexerTest, WhitespaceDelimiters) {
  std::stringstream ss;
  ss << "sam martha";

  Lexer lexer(&ss);

  EXPECT_THAT(lexer.token().type, Eq(Token::kIdentifier));
  EXPECT_THAT(lexer.token().value.string_value, Eq("sam"));
  lexer.Next();
  EXPECT_THAT(lexer.token().type, Eq(Token::kIdentifier));
  EXPECT_THAT(lexer.token().value.string_value, Eq("martha"));
  lexer.Next();
  EXPECT_THAT(lexer.token().type, Eq(Token::kEof));
}

TEST(LexerTest, NumericLiterals) {
  std::stringstream ss;
  ss << "1.125";

  Lexer lexer(&ss);

  EXPECT_THAT(lexer.token().type, Eq(Token::kNumber));
  EXPECT_THAT(lexer.token().value.double_value, Eq(1.125));
}

TEST(LexerTest, ReservedWords) {
  std::stringstream ss;
  ss << "def defy extern external if";

  Lexer lexer(&ss);

  EXPECT_THAT(lexer.token().type, Eq(Token::kDef));
  lexer.Next();

  EXPECT_THAT(lexer.token().type, Eq(Token::kIdentifier));
  EXPECT_THAT(lexer.token().value.string_value, Eq("defy"));
  lexer.Next();

  EXPECT_THAT(lexer.token().type, Eq(Token::kExtern));
  lexer.Next();

  EXPECT_THAT(lexer.token().type, Eq(Token::kIdentifier));
  EXPECT_THAT(lexer.token().value.string_value, Eq("external"));
  lexer.Next();

  EXPECT_THAT(lexer.token().type, Eq(Token::kIf));
}

TEST(LexerTest, LexDefintion) {
  std::stringstream ss;
  ss << "(def f (x y) (+ x 0.5))";

  Lexer lexer(&ss);

  std::vector<std::string> ids{"f", "x", "y", "x"};
  std::vector<double> nums{0.5};
  std::vector<char> types{'(',
                          Token::kDef,
                          Token::kIdentifier,
                          '(',
                          Token::kIdentifier,
                          Token::kIdentifier,
                          ')',
                          '(',
                          '+',
                          Token::kIdentifier,
                          Token::kNumber,
                          ')',
                          ')'};

  auto t = types.begin();
  auto i = ids.begin();
  auto n = nums.begin();

  const Token &tok = lexer.token();
  while (tok.type != Token::kEof) {
    EXPECT_TRUE(t != types.end());
    EXPECT_THAT(tok.type, Eq(*t));
    ++t;
    if (tok.type == Token::kIdentifier) {
      EXPECT_TRUE(i != ids.end());
      EXPECT_THAT(tok.value.string_value, Eq(*i));
      ++i;
    } else if (tok.type == Token::kNumber) {
      EXPECT_TRUE(n != nums.end());
      EXPECT_THAT(tok.value.double_value, Eq(*n));
      ++n;
    }
    lexer.Next();
  }

  EXPECT_TRUE(t == types.end());
  EXPECT_TRUE(i == ids.end());
  EXPECT_TRUE(n == nums.end());
}

} // namespace
} // namespace benscope